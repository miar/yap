#include <set>
#include <vector>
#include <algorithm>

#include <iostream>
#include <fstream>
#include <sstream>

#include "FactorGraph.h"
#include "Factor.h"
#include "BayesNet.h"
#include "BayesBall.h"
#include "Util.h"


bool FactorGraph::orderFactorVariables = false;


FactorGraph::FactorGraph (const FactorGraph& fg)
{
  const VarNodes& varNodes = fg.varNodes();
  for (unsigned i = 0; i < varNodes.size(); i++) {
    addVarNode (new VarNode (varNodes[i]));
  }
  const FacNodes& facNodes = fg.facNodes();
  for (unsigned i = 0; i < facNodes.size(); i++) {
    FacNode* facNode = new FacNode (facNodes[i]->factor());
    addFacNode (facNode);
    const VarNodes& neighs = facNodes[i]->neighbors();
    for (unsigned j = 0; j < neighs.size(); j++) {
      addEdge (varNodes_[neighs[j]->getIndex()], facNode);
    }
  }
}



void
FactorGraph::readFromUaiFormat (const char* fileName)
{
  std::ifstream is (fileName);
  if (!is.is_open()) {
    cerr << "error: cannot read from file " << fileName << endl;
    abort();
  }
  ignoreLines (is);
  string line;
  getline (is, line);
  if (line != "MARKOV") {
    cerr << "error: the network must be a MARKOV network " << endl;
    abort();
  }
  // read the number of vars
  ignoreLines (is);
  unsigned nrVars;
  is >> nrVars;
  // read the range of each var
  ignoreLines (is);
  Ranges ranges (nrVars);
  for (unsigned i = 0; i < nrVars; i++) {
    is >> ranges[i];
  }
  unsigned nrFactors;
  unsigned nrArgs;
  unsigned vid;
  is >> nrFactors;
  vector<VarIds> factorVarIds;
  vector<Ranges> factorRanges;
  for (unsigned i = 0; i < nrFactors; i++) {
    ignoreLines (is);
    is >> nrArgs;
    factorVarIds.push_back ({ });
    factorRanges.push_back ({ });
    for (unsigned j = 0; j < nrArgs; j++) {
      is >> vid;
      if (vid >= ranges.size()) {
        cerr << "error: invalid variable identifier `" << vid << "'" << endl;
        cerr << "identifiers must be between 0 and " << ranges.size() - 1 ;
        cerr << endl;
        abort();
      }
      factorVarIds.back().push_back (vid);
      factorRanges.back().push_back (ranges[vid]);
    }
  }
  // read the parameters
  unsigned nrParams;
  for (unsigned i = 0; i < nrFactors; i++) {
    ignoreLines (is);
    is >> nrParams;
    if (nrParams != Util::expectedSize (factorRanges[i])) {
      cerr << "error: invalid number of parameters for factor nº " << i ;
      cerr << ", expected: " << Util::expectedSize (factorRanges[i]);
      cerr << ", given: " << nrParams << endl;
      abort();
    }
    Params params (nrParams);
    for (unsigned j = 0; j < nrParams; j++) {
      is >> params[j];
    }
    if (Globals::logDomain) {
      Util::toLog (params);
    }
    addFactor (Factor (factorVarIds[i], factorRanges[i], params));
  }
  is.close();
}



void
FactorGraph::readFromLibDaiFormat (const char* fileName)
{
  std::ifstream is (fileName);
  if (!is.is_open()) {
    cerr << "error: cannot read from file " << fileName << endl;
    abort();
  }
  ignoreLines (is);
  unsigned nrFactors;
  unsigned nrArgs;
  VarId vid;
  is >> nrFactors;
  for (unsigned i = 0; i < nrFactors; i++) {
    ignoreLines (is);
    // read the factor arguments
    is >> nrArgs;
    VarIds vids;
    for (unsigned j = 0; j < nrArgs; j++) {
      ignoreLines (is);
      is >> vid;
      vids.push_back (vid);
    }
    // read ranges
    Ranges ranges (nrArgs);
    for (unsigned j = 0; j < nrArgs; j++) {
      ignoreLines (is);
      is >> ranges[j];
      VarNode* var = getVarNode (vids[j]);
      if (var != 0 && ranges[j] != var->range()) {
        cerr << "error: variable `" << vids[j] << "' appears in two or " ;
        cerr << "more factors with a different range" << endl;
      }
    }
    // read parameters
    ignoreLines (is);
    unsigned nNonzeros;
    is >> nNonzeros;
    Params params (Util::expectedSize (ranges), 0);
    for (unsigned j = 0; j < nNonzeros; j++) {
      ignoreLines (is);
      unsigned index;
      is >> index;
      ignoreLines (is);
      double val;
      is >> val;
      params[index] = val;
    }
    reverse (vids.begin(), vids.end());
    if (Globals::logDomain) {
      Util::toLog (params);
    }
    addFactor (Factor (vids, ranges, params));
  }
  is.close();
}



FactorGraph::~FactorGraph (void)
{
  for (unsigned i = 0; i < varNodes_.size(); i++) {
    delete varNodes_[i];
  }
  for (unsigned i = 0; i < facNodes_.size(); i++) {
    delete facNodes_[i];
  }
}



void
FactorGraph::addFactor (const Factor& factor)
{
  FacNode* fn = new FacNode (factor);
  addFacNode (fn);
  const VarIds& vids = factor.arguments();
  for (unsigned i = 0; i < vids.size(); i++) {
    bool found = false;
    for (unsigned j = 0; j < varNodes_.size(); j++) {
      if (varNodes_[j]->varId() == vids[i]) {
        addEdge (varNodes_[j], fn);
        found = true;
      }
    }
    if (found == false) {
      VarNode* vn = new VarNode (vids[i], factor.range (i));
      addVarNode (vn);
      addEdge (vn, fn);
    }
  }
}



void
FactorGraph::addVarNode (VarNode* vn)
{
  varNodes_.push_back (vn);
  vn->setIndex (varNodes_.size() - 1);
  varMap_.insert (make_pair (vn->varId(), vn));
}



void
FactorGraph::addFacNode (FacNode* fn)
{
  facNodes_.push_back (fn);
  fn->setIndex (facNodes_.size() - 1);
}



void
FactorGraph::addEdge (VarNode* vn, FacNode* fn)
{
  vn->addNeighbor (fn);
  fn->addNeighbor (vn);
}



bool
FactorGraph::isTree (void) const
{
  return !containsCycle();
}



DAGraph&
FactorGraph::getStructure (void)
{
  assert (fromBayesNet_);
  if (structure_.empty()) {
    for (unsigned i = 0; i < varNodes_.size(); i++) {
      structure_.addNode (new DAGraphNode (varNodes_[i]));
    }
    for (unsigned i = 0; i < facNodes_.size(); i++) {
      const VarIds& vids = facNodes_[i]->factor().arguments();
      for (unsigned j = 1; j < vids.size(); j++) {
        structure_.addEdge (vids[j], vids[0]);
      }
    }
  }
  return structure_;
}



void
FactorGraph::print (void) const
{
  for (unsigned i = 0; i < varNodes_.size(); i++) {
    cout << "var id   = " << varNodes_[i]->varId() << endl;
    cout << "label    = " << varNodes_[i]->label() << endl;
    cout << "range    = " << varNodes_[i]->range() << endl;
    cout << "evidence = " << varNodes_[i]->getEvidence() << endl;
    cout << "factors  = " ;
    for (unsigned j = 0; j < varNodes_[i]->neighbors().size(); j++) {
      cout << varNodes_[i]->neighbors()[j]->getLabel() << " " ;
    }
    cout << endl << endl;
  }
  for (unsigned i = 0; i < facNodes_.size(); i++) {
    facNodes_[i]->factor().print();
  }
}



void
FactorGraph::exportToGraphViz (const char* fileName) const
{
  ofstream out (fileName);
  if (!out.is_open()) {
    cerr << "error: cannot open file to write at " ;
    cerr << "FactorGraph::exportToDotFile()" << endl;
    abort();
  }
  out << "graph \"" << fileName << "\" {" << endl;
  for (unsigned i = 0; i < varNodes_.size(); i++) {
    if (varNodes_[i]->hasEvidence()) {
      out << '"' << varNodes_[i]->label() << '"' ;
      out << " [style=filled, fillcolor=yellow]" << endl;
    }
  }
  for (unsigned i = 0; i < facNodes_.size(); i++) {
    out << '"' << facNodes_[i]->getLabel() << '"' ;
    out << " [label=\"" << facNodes_[i]->getLabel(); 
    out << "\"" << ", shape=box]" << endl;
  }
  for (unsigned i = 0; i < facNodes_.size(); i++) {
    const VarNodes& myVars = facNodes_[i]->neighbors();
    for (unsigned j = 0; j < myVars.size(); j++) {
      out << '"' << facNodes_[i]->getLabel() << '"' ;
      out << " -- " ;
      out << '"' << myVars[j]->label() << '"' << endl;
    }
  }
  out << "}" << endl;
  out.close();
}



void
FactorGraph::exportToUaiFormat (const char* fileName) const
{
  ofstream out (fileName);
  if (!out.is_open()) {
    cerr << "error: cannot open file " << fileName << endl;
    abort();
  }
  out << "MARKOV" << endl;
  out << varNodes_.size() << endl;
  for (unsigned i = 0; i < varNodes_.size(); i++) {
    out << varNodes_[i]->range() << " " ;
  }
  out << endl;
  out << facNodes_.size() << endl;
  for (unsigned i = 0; i < facNodes_.size(); i++) {
    const VarNodes& factorVars = facNodes_[i]->neighbors();
    out << factorVars.size();
    for (unsigned j = 0; j < factorVars.size(); j++) {
      out << " " << factorVars[j]->getIndex();
    }
    out << endl;
  }
  for (unsigned i = 0; i < facNodes_.size(); i++) {
    Params params = facNodes_[i]->factor().params();
    if (Globals::logDomain) {
      Util::fromLog (params);
    }
    out << endl << params.size() << endl << " " ;
    for (unsigned j = 0; j < params.size(); j++) {
      out << params[j] << " " ;
    }
    out << endl;
  }
  out.close();
}



void
FactorGraph::exportToLibDaiFormat (const char* fileName) const
{
  ofstream out (fileName);
  if (!out.is_open()) {
    cerr << "error: cannot open file " << fileName << endl;
    abort();
  }
  out << facNodes_.size() << endl << endl;
  for (unsigned i = 0; i < facNodes_.size(); i++) {
    const VarNodes& factorVars = facNodes_[i]->neighbors();
    out << factorVars.size() << endl;
    for (int j = factorVars.size() - 1; j >= 0; j--) {
      out << factorVars[j]->varId() << " " ;
    }
    out << endl;
    for (unsigned j = 0; j < factorVars.size(); j++) {
      out << factorVars[j]->range() << " " ;
    }
    out << endl;
    Params params = facNodes_[i]->factor().params();
    if (Globals::logDomain) {
      Util::fromLog (params);
    }
    out << params.size() << endl;
    for (unsigned j = 0; j < params.size(); j++) {
      out << j << " " << params[j] << endl;
    }
    out << endl;
  }
  out.close();
}



void
FactorGraph::ignoreLines (std::ifstream& is) const
{
  string ignoreStr;
  while (is.peek() == '#' || is.peek() == '\n') {
    getline (is, ignoreStr);
  }
}



bool
FactorGraph::containsCycle (void) const
{
  vector<bool> visitedVars (varNodes_.size(), false);
  vector<bool> visitedFactors (facNodes_.size(), false);
  for (unsigned i = 0; i < varNodes_.size(); i++) {
    int v = varNodes_[i]->getIndex();
    if (!visitedVars[v]) {
      if (containsCycle (varNodes_[i], 0, visitedVars, visitedFactors)) {
        return true;
      }
    }
  }
  return false;
}



bool
FactorGraph::containsCycle (
    const VarNode* v,
    const FacNode* p,
    vector<bool>& visitedVars,
    vector<bool>& visitedFactors) const
{
  visitedVars[v->getIndex()] = true;
  const FacNodes& adjacencies = v->neighbors();
  for (unsigned i = 0; i < adjacencies.size(); i++) {
    int w = adjacencies[i]->getIndex();
    if (!visitedFactors[w]) {
      if (containsCycle (adjacencies[i], v, visitedVars, visitedFactors)) {
        return true;
      }
    }
    else if (visitedFactors[w] && adjacencies[i] != p) {
      return true;
    }
  }
  return false; // no cycle detected in this component
}



bool
FactorGraph::containsCycle (
    const FacNode* v,
    const VarNode* p,
    vector<bool>& visitedVars,
    vector<bool>& visitedFactors) const
{
  visitedFactors[v->getIndex()] = true;
  const VarNodes& adjacencies = v->neighbors();
  for (unsigned i = 0; i < adjacencies.size(); i++) {
    int w = adjacencies[i]->getIndex();
    if (!visitedVars[w]) {
      if (containsCycle (adjacencies[i], v, visitedVars, visitedFactors)) {
        return true;
      }
    }
    else if (visitedVars[w] && adjacencies[i] != p) {
      return true;
    }
  }
  return false; // no cycle detected in this component
}

