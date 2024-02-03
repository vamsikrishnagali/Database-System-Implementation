#include <fstream>

#include "Errors.h"
#include "DBFile.h"
#include "HeapFile.h"

using std::string;
using std::ofstream;

int DBFile::Create (char* fpath, fType ftype, void* startup) {
  FATALIF(db!=NULL, "File already opened.");
  createFile(ftype);
  return db->Create(fpath, startup);
}

int DBFile::Open (char* fpath) {
  FATALIF(db!=NULL, "File already opened.");
  int ftype = heap;  // use heap file by default
  ifstream ifs((GenericDBFile::getTableName(fpath)+".meta").c_str());
  if (ifs) {
    ifs >> ftype;  // the first line contains file type
    ifs.close();
  }
  createFile(static_cast<fType>(ftype));
  return db->Open(fpath);
}

void DBFile::createFile(fType ftype) {
  switch (ftype) {
    case heap: db = new HeapFile; break;
    default: db = NULL;
  }
  FATALIF(db==NULL, "Invalid file type.");
}

int DBFile::Close() {
  return db->Close();
}

void DBFile::Add (Record& addme) {
  return db->Add(addme);
}

void DBFile::Load (Schema& myschema, char* loadpath) {
  return db->Load(myschema, loadpath);
}

void DBFile::MoveFirst() {
  return db->MoveFirst();
}

int DBFile::GetNext (Record& fetchme) {
  return db->GetNext(fetchme);
}

int DBFile::GetNext (Record& fetchme, CNF& cnf, Record& literal) { 
  return db->GetNext(fetchme, cnf, literal);
}

DBFile::DBFile(): db(NULL) {}
DBFile::~DBFile() { delete db; }

int GenericDBFile::Create(char* fpath, void* startup) {
  theFile.Open(0, fpath);
  return 1;
}

int GenericDBFile::Open (char* fpath) {
  theFile.Open(1, fpath);
  return 1;
}

void GenericDBFile::Load (Schema& myschema, char* loadpath) {
  startWrite();
  FILE* ifp = fopen(loadpath, "r");
  FATALIF(ifp==NULL, loadpath);

  Record next;
  curPage.EmptyItOut();  // creates the first page
  while (next.SuckNextRecord(&myschema, ifp)) Add(next);
}

int GenericDBFile::GetNext (Record& fetchme) {
  while (!curPage.GetFirst(&fetchme)) {
    if(++curPageIdx > theFile.lastIndex()) return 0;  // no more records
    theFile.GetPage(&curPage, curPageIdx);
  }
  return 1;
}
