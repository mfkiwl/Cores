#ifndef _PROTO_H
#define _PROTO_H

// Register.cpp
bool IsArgumentReg(int regno);
bool IsCalleeSave(int regno);

int64_t GetConstExpression(ENODE **pnode);
void GenMemop(int op, Operand *ap1, Operand *ap2, int ssize);
void GenerateHint(int num);

void SaveRegisterVars(int64_t mask, int64_t rmask);
void SaveFPRegisterVars(int64_t fpmask, int64_t fprmask);
void GenLdi(Operand *, Operand *);
void SaveRegisterVars(int64_t mask, int64_t rmask);
void SaveFPRegisterVars(int64_t mask, int64_t rmask);
void funcbottom(Statement *stmt);
Function *allocFunction(int id);
SYM *makeint2(std::string na);
int round8(int n);
int pwrof2(int64_t);
void ListCompound(Statement *stmt);
std::string TraceName(SYM *sp);
void MarkRemove(OCODE *ip);
void IRemove();
int roundSize(TYP *tp);
extern char *rtrim(char *);
extern int caselit(scase *casetab, int64_t);
Operand *make_indexed2(int lab, int i);

// MemoryManagement.cpp
void FreeFunction(Function *fn);

// Outcode.cpp
extern void genstorage(int64_t nbytes);
extern void GenerateByte(int64_t val);
extern void GenerateChar(int64_t val);
extern void genhalf(int64_t val);
extern void GenerateWord(int64_t val);
extern void GenerateLong(int64_t val);
extern void GenerateFloat(Float128 *val);
extern void GenerateQuad(Float128 *);
extern void GenerateReference(SYM *sp, int64_t offset);
extern void GenerateLabelReference(int n);

extern char *RegMoniker(int regno);

#endif