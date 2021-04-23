#include <algorithm>
#include <ostream>
#include "3ac.hpp"
#include "err.hpp"

namespace holeyc{

	// Give a label to each global
	// E.g. g1->myLoc = gbl_g1;
void IRProgram::allocGlobals(){
	// Get all global values, then set their labels
	auto globals = getGlobals();
	for(auto global : globals)
		global->setMemoryLoc("gbl_" + global->getName());

	// Get all strings (and corresponding AuxOpd*'s) and set their labels
	auto stringAuxs = getAuxForStrings();
	auto strings = getStrings();
	size_t itr = 0;
	for(auto aux_itr = stringAuxs.begin(); aux_itr != stringAuxs.end(); aux_itr++){
		// Omit quotation marks from string
		std::string tmp = "str_" + strings[itr].substr(1, strings[itr].length()-2);
		// Remove all spaces from the string
		tmp.erase(std::remove_if(tmp.begin(), tmp.end(), ::isspace), tmp.end()); // remove all spaces
		tmp.erase(std::remove_if(tmp.begin(), tmp.end(), ::ispunct), tmp.end()); // remove all punctuation
		(*aux_itr)->setMemoryLoc(tmp);
		itr++; // stringAuxs and strings will always be the same size since
								// they both came from the same std::map
	}
}

// Write out the .data section with .asciz's and .quad's
void IRProgram::datagenX64(std::ostream& out){
	// Set the memory locations of all global values
	allocGlobals();

	// Print the data section
	out << ".data\n";
	// Get the global values and print their labels with the .quad directive
	auto globals = getGlobals();
	for(auto global : globals)
		out << global->getMemoryLoc() << ":\n"
			<< "   .quad 0\n";
	// Get all string values and print their labels with the .asciz directive
	auto stringAuxs = getAuxForStrings();
	auto strs = getStrings();
	size_t itr = 0;
	for(auto str : stringAuxs){
		out << str->getMemoryLoc() << ":\n"
			<< ".asciz " << strs[itr] << "\n";
		itr++;
	}
	//Put this directive after you write out strings
	// so that everything is aligned to a quadword value
	// again
	out << ".align 8\n\n";
}

// Go through the list of procedures
void IRProgram::toX64(std::ostream& out){
	// Handle the data section first
	datagenX64(out);
	//Then handle procedures
	out << ".text\n"
		<< ".globl main\n";
	auto procs = getProcs();
	for(auto proc : procs)
		proc->toX64(out);
}

void Procedure::allocLocals(){
	// Data generation pass
	auto allLocals = getLocals();
	int offset = -24;
	for(auto local : allLocals){
		local->setMemoryLoc(std::to_string(offset) + "(%rbp)");
		offset -= 8;
	}

	auto allTemps = getTemps();
	for(auto tmp : allTemps){
		tmp->setMemoryLoc(std::to_string(offset) + "(%rbp)");
		offset -= 8;
	}

	for(auto formal : formals){
		formal->setMemoryLoc(std::to_string(offset) + "(%rbp)");
		offset -= 8;
	}
}

void Procedure::toX64(std::ostream& out){
	// Code pass
	//Allocate all locals
	allocLocals();

	if (myName == "main"){
		out << "main:\n";
	}
	else{
		enter->codegenLabels(out);
		out << '\n';
	}
	enter->codegenX64(out);
	for (auto quad : *bodyQuads){
		quad->codegenLabels(out);
		quad->codegenX64(out);
	}
	leave->codegenLabels(out);
	leave->codegenX64(out);
}

void Quad::codegenLabels(std::ostream& out){
	if (labels.empty()){ return; }

	size_t numLabels = labels.size();
	size_t labelIdx = 0;
	for ( Label * label : labels){
		out << label->toString() << ": ";
		if (labelIdx != numLabels - 1){ out << "\n"; }
		labelIdx++;
	}
}

void BinOpQuad::codegenX64(std::ostream& out){
	src1->genLoad(out, "%rax");
	src2->genLoad(out, "%rbx");
	switch(op){
		case ADD: out << "addq %rbx, %rax\n";
							dst->genStore(out, "%rax");
							break;
		case SUB: out << "subq %rbx, %rax\n";
							dst->genStore(out, "%rax");
							break;
		case DIV: out << "movq $0, %rdx\n"
									<< "idivq %rbx\n";
							dst->genStore(out, "%rax");
							break;
		case MULT: out << "imulq %rbx\n";
							 dst->genStore(out, "%rax");
							 break;
		case OR: out << "orq %rbx, %rax\n";
						 dst->genStore(out, "%rax");
						 break;
		case AND: out << "andq %rbx, %rax\n";
							dst->genStore(out, "%rax");
							break;
		case EQ: out << "cmpq %rbx, %rax\n"
								 << "sete %al\n"
						 		 << "movb %al, " << dst->getMemoryLoc() << '\n';
						 break;
		case NEQ: out << "cmpq %rbx, %rax\n"
								 << "setne %al\n"
						 		 << "movb %al, " << dst->getMemoryLoc() << '\n';
						 break;
		case LT: out << "cmpq %rbx, %rax\n"
								 << "setl %al\n"
						 		 << "movb %al, " << dst->getMemoryLoc() << '\n';
						 break;
		case GT: out << "cmpq %rbx, %rax\n"
								 << "setg %al\n"
						 		 << "movb %al, " << dst->getMemoryLoc() << '\n';
						 break;
		case LTE: out << "cmpq %rbx, %rax\n"
								 << "setle %al\n"
						 		 << "movb %al, " << dst->getMemoryLoc() << '\n';
						 break;
		case GTE: out << "cmpq %rbx, %rax\n"
								 << "setge %al\n"
						 		 << "movb %al, " << dst->getMemoryLoc() << '\n';
						 break;
	}
}

void UnaryOpQuad::codegenX64(std::ostream& out){
	src->genLoad(out, "%rax");
	switch(op){
		case NEG: out << "negq %rax\n";
							dst->genStore(out, "%rax");
							break;
		case NOT: out << "movq $1, %rbx\n"
									<< "xorq %rbx, %rax\n";
						  dst->genStore(out, "%rax");
						  break;
	}
}

void AssignQuad::codegenX64(std::ostream& out){
	src->genLoad(out, "%rax");
	dst->genStore(out, "%rax");
}

void LocQuad::codegenX64(std::ostream& out){
	// (Optional)
	// TODO(Implement me)
}

void JmpQuad::codegenX64(std::ostream& out){
	out << "jmp " << tgt->toString() << "\n";
}

void JmpIfQuad::codegenX64(std::ostream& out){
	cnd->genLoad(out, "%rax");
	out << "cmpq $0, %rax\n"
		<< "je " << tgt->toString() << '\n';
}

void NopQuad::codegenX64(std::ostream& out){
	out << "nop\n";
}

void IntrinsicQuad::codegenX64(std::ostream& out){
	switch(myIntrinsic){
	case OUTPUT:
		if(myArg->getWidth() == ADDR){
			out << "movq $" << myArg->getMemoryLoc() << ", %rdi\n";
		}
		else{
			myArg->genLoad(out, "%rdi");
		}
		if (myArg->getWidth() == QUADWORD){
			out << "callq printInt\n";
		} else if (myArg->getWidth() == BYTE){
			out << "callq printByte\n";
		} else {
			//If the argument is an ADDR,
			// assume it's a string
			out << "callq printString\n";
		}
		break;
	case INPUT:
		if(myArg->getWidth() == QUADWORD){
			out << "callq getInt\n";
		} else if(myArg->getWidth() == BYTE){
			if(myArg->valString().length() > 1){ // TODO
				out << "callq getBool\n";
			}
			else{
				out << "callq getChar\n";
			}
		} else{
			out << "callq getChar\n";
		}
		myArg->genStore(out, "%rax");
	}
}

void CallQuad::codegenX64(std::ostream& out){
	out << "callq " << "lbl_fun_" + callee->getName() << '\n';
}

void EnterQuad::codegenX64(std::ostream& out){
	int size = myProc->localsSize();
	size += (size % 16);
	out << "pushq %rbp\n"
		<< "movq %rsp, %rbp\n"
		<< "addq $16, %rbp\n"
		<< "subq $" << size << ", %rsp\n";
}

void LeaveQuad::codegenX64(std::ostream& out){
	int size = myProc->localsSize();
	size += (size % 16);
	out << "addq $" << size << ", %rsp\n"
		<< "popq %rbp\n"
		<< "retq\n";
}

void SetArgQuad::codegenX64(std::ostream& out){
	if(index == 1){
		opd->genLoad(out, "%rdi");
	} else if(index == 2){
		opd->genLoad(out, "%rsi");
	} else if(index == 3){
		opd->genLoad(out, "%rdx");
	} else if(index == 4){
		opd->genLoad(out, "%rcx");
	} else if(index == 5){
		opd->genLoad(out, "%r8");
	} else if(index == 6){
		opd->genLoad(out, "%r9");
	} else{
		out << "subq $8, %rsp\n"; // make space on the stack
		opd->genLoad(out, opd->getMemoryLoc());
	}
}

void GetArgQuad::codegenX64(std::ostream& out){
	if(index == 1){
		opd->genStore(out, "%rdi");
	} else if(index == 2){
		opd->genStore(out, "%rsi");
	} else if(index == 3){
		opd->genStore(out, "%rdx");
	} else if(index == 4){
		opd->genStore(out, "%rcx");
	} else if(index == 5){
		opd->genStore(out, "%r8");
	} else if(index == 6){
		opd->genStore(out, "%r9");
	} else{
		// No action needed; HoleyC is always pass-by-value, so we can target
			// these arguments directly within the stack -- ergo no registers needed
	}
}

void SetRetQuad::codegenX64(std::ostream& out){
	std::string tmp = '$' + opd->valString();
	if(tmp[1] == '[') tmp = opd->getMemoryLoc();
	out << "movq " << tmp << ", %rax\n";
}

void GetRetQuad::codegenX64(std::ostream& out){
	opd->genStore(out, "%rax");
}

void SymOpd::genLoad(std::ostream & out, std::string regStr){
	out << "movq " << getMemoryLoc() << ", " << regStr << '\n';
}

void SymOpd::genStore(std::ostream& out, std::string regStr){
	out << "movq " << regStr << ", " << getMemoryLoc() << "\n";
}

void AuxOpd::genLoad(std::ostream & out, std::string regStr){
	out << "movq " << getMemoryLoc() << ", " << regStr << '\n';
}

void AuxOpd::genStore(std::ostream& out, std::string regStr){
	out << "movq " << regStr << ", " << getMemoryLoc() << '\n';
}

void LitOpd::genLoad(std::ostream & out, std::string regStr){
	if(getWidth() == BYTE){ // the literal is a character
		if(int(val[0]) == 48){
			out << "movq $0, " << regStr << '\n';
		}
		else if(int(val[1]) == 49){
			out << "movq $1, " << regStr << '\n';
		}
		else{
			out << "movq $" << int(val[0]) << ", " << regStr << '\n';
		}
	}
	else{
		out << "movq " << getMemoryLoc() << ", " << regStr << '\n';
	}
}

void LitOpd::genStore(std::ostream& out, std::string regStr){
	throw new InternalError("Cannot use literal as l-val");
}

}
