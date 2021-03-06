#include <cstdlib>

#include <vector>

#include <iostream>
#include <sstream>

#include <YapInterface.h>

#include "ParfactorList.h"
#include "FactorGraph.h"
#include "FoveSolver.h"
#include "VarElimSolver.h"
#include "BpSolver.h"
#include "CbpSolver.h"
#include "ElimGraph.h"
#include "BayesBall.h"


using namespace std;


typedef std::pair<ParfactorList*, ObservedFormulas*> LiftedNetwork;


Params readParameters (YAP_Term);

vector<unsigned> readUnsignedList (YAP_Term);

void readLiftedEvidence (YAP_Term, ObservedFormulas&);

Parfactor* readParfactor (YAP_Term);

void runVeSolver (FactorGraph* fg, const vector<VarIds>& tasks,
    vector<Params>& results);

void runBpSolver (FactorGraph* fg, const vector<VarIds>& tasks,
   vector<Params>& results);




vector<unsigned>
readUnsignedList (YAP_Term list)
{
  vector<unsigned> vec;
  while (list != YAP_TermNil()) {
    vec.push_back ((unsigned) YAP_IntOfTerm (YAP_HeadOfTerm (list)));
    list = YAP_TailOfTerm (list);
  }
  return vec;
}



int createLiftedNetwork (void)
{
  Parfactors parfactors;
  YAP_Term parfactorList = YAP_ARG1;
  while (parfactorList != YAP_TermNil()) {
    YAP_Term pfTerm = YAP_HeadOfTerm (parfactorList);
    parfactors.push_back (readParfactor (pfTerm));
	  parfactorList = YAP_TailOfTerm (parfactorList);
  }

  // LiftedUtils::printSymbolDictionary();
  if (Constants::DEBUG > 2) {
    // Util::printHeader ("INITIAL PARFACTORS");
    // for (unsigned i = 0; i < parfactors.size(); i++) {
    //  parfactors[i]->print();
    // }
  }

  ParfactorList* pfList = new ParfactorList (parfactors);

  if (Constants::DEBUG >= 2) {
    Util::printHeader ("SHATTERED PARFACTORS");
    pfList->print();
  }

  // read evidence
  ObservedFormulas* obsFormulas = new ObservedFormulas();
  readLiftedEvidence (YAP_ARG2, *(obsFormulas));

  LiftedNetwork* net = new LiftedNetwork (pfList, obsFormulas);
  YAP_Int p = (YAP_Int) (net);
  return YAP_Unify (YAP_MkIntTerm (p), YAP_ARG3);
}



Parfactor* readParfactor (YAP_Term pfTerm)
{
  // read dist id
  unsigned distId = YAP_IntOfTerm (YAP_ArgOfTerm (1, pfTerm));

  // read the ranges
  Ranges ranges;
  YAP_Term rangeList = YAP_ArgOfTerm (3, pfTerm);
  while (rangeList != YAP_TermNil()) {
    unsigned range = (unsigned) YAP_IntOfTerm (YAP_HeadOfTerm (rangeList));
    ranges.push_back (range);
    rangeList = YAP_TailOfTerm (rangeList);
  }

  // read parametric random vars
  ProbFormulas formulas;
  unsigned count = 0;
  unordered_map<YAP_Term, LogVar> lvMap;
  YAP_Term pvList = YAP_ArgOfTerm (2, pfTerm);
  while (pvList != YAP_TermNil()) {
    YAP_Term formulaTerm = YAP_HeadOfTerm (pvList);
    if (YAP_IsAtomTerm (formulaTerm)) {
      string name ((char*) YAP_AtomName (YAP_AtomOfTerm (formulaTerm)));
      Symbol functor = LiftedUtils::getSymbol (name);
      formulas.push_back (ProbFormula (functor, ranges[count]));
    } else {
      LogVars logVars;
      YAP_Functor yapFunctor = YAP_FunctorOfTerm (formulaTerm);
      string name ((char*) YAP_AtomName (YAP_NameOfFunctor (yapFunctor)));
      Symbol functor = LiftedUtils::getSymbol (name);
      unsigned arity = (unsigned) YAP_ArityOfFunctor (yapFunctor);
      for (unsigned i = 1; i <= arity; i++) {
        YAP_Term ti = YAP_ArgOfTerm (i, formulaTerm);
        unordered_map<YAP_Term, LogVar>::iterator it = lvMap.find (ti);
        if (it != lvMap.end()) {
          logVars.push_back (it->second);
        } else {
         unsigned newLv = lvMap.size();
         lvMap[ti] = newLv;
         logVars.push_back (newLv);
        }
      }
      formulas.push_back (ProbFormula (functor, logVars, ranges[count]));
    }
    count ++;
    pvList = YAP_TailOfTerm (pvList);
  }

  // read the parameters
  const Params& params = readParameters (YAP_ArgOfTerm (4, pfTerm)); 

  // read the constraint
  Tuples tuples;
  if (lvMap.size() >= 1) {
    YAP_Term tupleList = YAP_ArgOfTerm (5, pfTerm);
    while (tupleList != YAP_TermNil()) {
      YAP_Term term = YAP_HeadOfTerm (tupleList);
      assert (YAP_IsApplTerm (term));
      YAP_Functor yapFunctor = YAP_FunctorOfTerm (term);
      unsigned arity = (unsigned) YAP_ArityOfFunctor (yapFunctor);
      assert (lvMap.size() == arity);
      Tuple tuple (arity);
      for (unsigned i = 1; i <= arity; i++) {
        YAP_Term ti = YAP_ArgOfTerm (i, term);
        if (YAP_IsAtomTerm (ti) == false) {
          cerr << "error: constraint has free variables" << endl;
          abort();
        }
        string name ((char*) YAP_AtomName (YAP_AtomOfTerm (ti)));
        tuple[i - 1] = LiftedUtils::getSymbol (name);
      }
      tuples.push_back (tuple);
      tupleList = YAP_TailOfTerm (tupleList);
    }
  }
  return new Parfactor (formulas, params, tuples, distId);
}



void readLiftedEvidence (
    YAP_Term observedList,
    ObservedFormulas& obsFormulas)
{
  while (observedList != YAP_TermNil()) {
    YAP_Term pair = YAP_HeadOfTerm (observedList);
    YAP_Term ground = YAP_ArgOfTerm (1, pair);
    Symbol functor;
    Symbols args;
    if (YAP_IsAtomTerm (ground)) {
      string name ((char*) YAP_AtomName (YAP_AtomOfTerm (ground)));
      functor = LiftedUtils::getSymbol (name);
    } else {
      assert (YAP_IsApplTerm (ground));
      YAP_Functor yapFunctor = YAP_FunctorOfTerm (ground);
      string name ((char*) (YAP_AtomName (YAP_NameOfFunctor (yapFunctor))));
      functor = LiftedUtils::getSymbol (name);
      unsigned arity = (unsigned) YAP_ArityOfFunctor (yapFunctor);
      for (unsigned i = 1; i <= arity; i++) {
        YAP_Term ti = YAP_ArgOfTerm (i, ground);
        assert (YAP_IsAtomTerm (ti));
        string arg ((char *) YAP_AtomName (YAP_AtomOfTerm (ti)));
        args.push_back (LiftedUtils::getSymbol (arg));
      }
    }
    unsigned evidence = (unsigned) YAP_IntOfTerm (YAP_ArgOfTerm (2, pair));
    bool found = false;
    for (unsigned i = 0; i < obsFormulas.size(); i++) {
      if (obsFormulas[i].functor()  == functor &&
          obsFormulas[i].arity()    == args.size() &&
          obsFormulas[i].evidence() == evidence) {
        obsFormulas[i].addTuple (args);
        found = true;
      }
    }
    if (found == false) {
      obsFormulas.push_back (ObservedFormula (functor, evidence, args));
    }
    observedList = YAP_TailOfTerm (observedList);
  } 
}



int
createGroundNetwork (void)
{
  string factorsType ((char*) YAP_AtomName (YAP_AtomOfTerm (YAP_ARG1)));
  bool fromBayesNet = factorsType == "bayes";
  FactorGraph* fg = new FactorGraph (fromBayesNet);
  YAP_Term factorList = YAP_ARG2;
  while (factorList != YAP_TermNil()) {
    YAP_Term factor = YAP_HeadOfTerm (factorList);
    // read the var ids
    VarIds varIds = readUnsignedList (YAP_ArgOfTerm (1, factor));
    // read the ranges
    Ranges ranges = readUnsignedList (YAP_ArgOfTerm (2, factor));
    // read the parameters
    Params params = readParameters (YAP_ArgOfTerm (3, factor)); 
    // read dist id
    unsigned distId = (unsigned) YAP_IntOfTerm (YAP_ArgOfTerm (4, factor));
    fg->addFactor (Factor (varIds, ranges, params, distId));
    factorList = YAP_TailOfTerm (factorList);
  }

  YAP_Term evidenceList = YAP_ARG3;
  while (evidenceList != YAP_TermNil()) {
    YAP_Term evTerm = YAP_HeadOfTerm (evidenceList);
    unsigned vid = (unsigned) YAP_IntOfTerm ((YAP_ArgOfTerm (1, evTerm)));
    unsigned ev  = (unsigned) YAP_IntOfTerm ((YAP_ArgOfTerm (2, evTerm)));
    assert (fg->getVarNode (vid));
    fg->getVarNode (vid)->setEvidence (ev);
    evidenceList = YAP_TailOfTerm (evidenceList);
  }

  YAP_Int p = (YAP_Int) (fg);
  return YAP_Unify (YAP_MkIntTerm (p), YAP_ARG4);
}



Params
readParameters (YAP_Term paramL)
{
  Params params;
  assert (YAP_IsPairTerm (paramL));
  while (paramL != YAP_TermNil()) {
    params.push_back ((double) YAP_FloatOfTerm (YAP_HeadOfTerm (paramL)));
    paramL = YAP_TailOfTerm (paramL);
  }
  if (Globals::logDomain) {
    Util::toLog (params);
  }
  return params;
}



int
runLiftedSolver (void)
{
  LiftedNetwork* network = (LiftedNetwork*) YAP_IntOfTerm (YAP_ARG1);
  YAP_Term taskList = YAP_ARG2;
  vector<Params> results;
  ParfactorList pfListCopy (*network->first);
  FoveSolver::absorveEvidence (pfListCopy, *network->second);
  while (taskList != YAP_TermNil()) {
    Grounds queryVars;
    YAP_Term jointList = YAP_HeadOfTerm (taskList);
    while (jointList != YAP_TermNil()) {
      YAP_Term ground = YAP_HeadOfTerm (jointList);
      if (YAP_IsAtomTerm (ground)) {
        string name ((char*) YAP_AtomName (YAP_AtomOfTerm (ground)));
        queryVars.push_back (Ground (LiftedUtils::getSymbol (name)));
      } else {
        assert (YAP_IsApplTerm (ground));
        YAP_Functor yapFunctor = YAP_FunctorOfTerm (ground);
        string name ((char*) (YAP_AtomName (YAP_NameOfFunctor (yapFunctor))));
        unsigned arity = (unsigned) YAP_ArityOfFunctor (yapFunctor);
        Symbol functor = LiftedUtils::getSymbol (name);
        Symbols args;
        for (unsigned i = 1; i <= arity; i++) {
          YAP_Term ti = YAP_ArgOfTerm (i, ground);
          assert (YAP_IsAtomTerm (ti));
          string arg ((char *) YAP_AtomName (YAP_AtomOfTerm (ti)));
          args.push_back (LiftedUtils::getSymbol (arg));
        }
        queryVars.push_back (Ground (functor, args));
      }
      jointList = YAP_TailOfTerm (jointList);
    }
    FoveSolver solver (pfListCopy);
    if (queryVars.size() == 1) {
      results.push_back (solver.getPosterioriOf (queryVars[0]));
    } else {
      results.push_back (solver.getJointDistributionOf (queryVars));
    }
    taskList = YAP_TailOfTerm (taskList);
  }

  YAP_Term list = YAP_TermNil();
  for (int i = results.size() - 1; i >= 0; i--) {
    const Params& beliefs = results[i];
    YAP_Term queryBeliefsL = YAP_TermNil();
    for (int j = beliefs.size() - 1; j >= 0; j--) {
      YAP_Int sl1      = YAP_InitSlot (list);
      YAP_Term belief  = YAP_MkFloatTerm (beliefs[j]);
      queryBeliefsL    = YAP_MkPairTerm (belief, queryBeliefsL);
      list             = YAP_GetFromSlot (sl1);
      YAP_RecoverSlots (1);
    }
    list = YAP_MkPairTerm (queryBeliefsL, list);
  }
 
  return YAP_Unify (list, YAP_ARG3);
}



int
runGroundSolver (void)
{
  FactorGraph* fg = (FactorGraph*) YAP_IntOfTerm (YAP_ARG1);

  vector<VarIds> tasks;
  YAP_Term taskList = YAP_ARG2;
  while (taskList != YAP_TermNil()) {
    tasks.push_back (readUnsignedList (YAP_HeadOfTerm (taskList)));
    taskList = YAP_TailOfTerm (taskList);
  }

  vector<Params> results;
  if (Globals::infAlgorithm == InfAlgorithms::VE) {
    runVeSolver (fg, tasks, results);
  } else {
    runBpSolver (fg, tasks, results);
  }

  YAP_Term list = YAP_TermNil();
  for (int i = results.size() - 1; i >= 0; i--) {
    const Params& beliefs = results[i];
    YAP_Term queryBeliefsL = YAP_TermNil();
    for (int j = beliefs.size() - 1; j >= 0; j--) {
      YAP_Int sl1      = YAP_InitSlot (list);
      YAP_Term belief  = YAP_MkFloatTerm (beliefs[j]);
      queryBeliefsL    = YAP_MkPairTerm (belief, queryBeliefsL);
      list             = YAP_GetFromSlot (sl1);
      YAP_RecoverSlots (1);
    }
    list = YAP_MkPairTerm (queryBeliefsL, list);
  }
  return YAP_Unify (list, YAP_ARG3);
}



void runVeSolver (
   FactorGraph* fg,
   const vector<VarIds>& tasks,
   vector<Params>& results) 
{
  results.reserve (tasks.size());
  for (unsigned i = 0; i < tasks.size(); i++) {
    FactorGraph* mfg = fg;
    if (fg->isFromBayesNetwork()) {
      mfg = BayesBall::getMinimalFactorGraph (*fg, tasks[i]);
    }
    VarElimSolver solver (*mfg);
    results.push_back (solver.solveQuery (tasks[i]));
    if (fg->isFromBayesNetwork()) {
      delete mfg;
    }
  }
}



void runBpSolver (
    FactorGraph* fg,
    const vector<VarIds>& tasks,
    vector<Params>& results) 
{
  std::set<VarId> vids;
  for (unsigned i = 0; i < tasks.size(); i++) {
    Util::addToSet (vids, tasks[i]);
  }
  Solver* solver = 0;
  FactorGraph* mfg = fg;
  if (fg->isFromBayesNetwork()) {
    mfg = BayesBall::getMinimalFactorGraph (
        *fg, VarIds (vids.begin(),vids.end()));
  }
  if (Globals::infAlgorithm == InfAlgorithms::BP) {
    solver = new BpSolver (*mfg);
  } else if (Globals::infAlgorithm == InfAlgorithms::CBP) {
    CFactorGraph::checkForIdenticalFactors = false;
    solver = new CbpSolver (*mfg);
  } else {
    cerr << "error: unknow solver" << endl;
    abort();
  }
  results.reserve (tasks.size());
  for (unsigned i = 0; i < tasks.size(); i++) {
    results.push_back (solver->solveQuery (tasks[i]));
  }
  if (fg->isFromBayesNetwork()) {
    delete mfg;
  }
  delete solver;
}



int
setParfactorsParams (void)
{
  LiftedNetwork* network = (LiftedNetwork*) YAP_IntOfTerm (YAP_ARG1);
  ParfactorList* pfList = network->first;
  YAP_Term distList = YAP_ARG2;
  unordered_map<unsigned, Params> paramsMap;
  while (distList != YAP_TermNil()) {
    YAP_Term dist   = YAP_HeadOfTerm (distList);
    unsigned distId = (unsigned) YAP_IntOfTerm (YAP_ArgOfTerm (1, dist));
    assert (Util::contains (paramsMap, distId) == false);
    paramsMap[distId] = readParameters (YAP_ArgOfTerm (2, dist));
    distList = YAP_TailOfTerm (distList);
  }
  ParfactorList::iterator it = pfList->begin();
  while (it != pfList->end()) {
    assert (Util::contains (paramsMap, (*it)->distId()));
    // (*it)->setParams (paramsMap[(*it)->distId()]); 
    ++ it;
  }
  return TRUE;
}



int
setFactorsParams (void)
{
  return TRUE; // TODO
  FactorGraph* fg = (FactorGraph*) YAP_IntOfTerm (YAP_ARG1);
  YAP_Term distList = YAP_ARG2;
  unordered_map<unsigned, Params> paramsMap;
  while (distList != YAP_TermNil()) {
    YAP_Term dist   = YAP_HeadOfTerm (distList);
    unsigned distId = (unsigned) YAP_IntOfTerm (YAP_ArgOfTerm (1, dist));
    assert (Util::contains (paramsMap, distId) == false);
    paramsMap[distId] = readParameters (YAP_ArgOfTerm (2, dist));
    distList = YAP_TailOfTerm (distList);
  }
  const FacNodes& facNodes = fg->facNodes();
  for (unsigned i = 0; i < facNodes.size(); i++) {
    unsigned distId = facNodes[i]->factor().distId();
    assert (Util::contains (paramsMap, distId));
    facNodes[i]->factor().setParams (paramsMap[distId]);
  }
  return TRUE;
}



int
setVarsInformation (void)
{
  Var::clearVarsInfo();
  YAP_Term labelsL = YAP_ARG1;
  vector<string> labels;
  while (labelsL != YAP_TermNil()) {
    YAP_Atom atom = YAP_AtomOfTerm (YAP_HeadOfTerm (labelsL));
    labels.push_back ((char*) YAP_AtomName (atom));
    labelsL = YAP_TailOfTerm (labelsL);
  }
  unsigned count = 0;
  YAP_Term stateNamesL = YAP_ARG2;
  while (stateNamesL != YAP_TermNil()) {
    States states;
    YAP_Term namesL = YAP_HeadOfTerm (stateNamesL);
    while (namesL != YAP_TermNil()) {
      YAP_Atom atom = YAP_AtomOfTerm (YAP_HeadOfTerm (namesL));
      states.push_back ((char*) YAP_AtomName (atom));
      namesL = YAP_TailOfTerm (namesL);
    }
    Var::addVarInfo (count, labels[count], states);
    count ++;
    stateNamesL = YAP_TailOfTerm (stateNamesL);
  }
  return TRUE;
}



int
setHorusFlag (void)
{
  string key ((char*) YAP_AtomName (YAP_AtomOfTerm (YAP_ARG1)));
  if (key == "inf_alg") {
    string value ((char*) YAP_AtomName (YAP_AtomOfTerm (YAP_ARG2)));
    if (       value == "ve") {
      Globals::infAlgorithm = InfAlgorithms::VE;
    } else if (value == "bp") {
      Globals::infAlgorithm = InfAlgorithms::BP;
    } else if (value == "cbp") {
      Globals::infAlgorithm = InfAlgorithms::CBP;
    } else {
      cerr << "warning: invalid value `" << value << "' " ;
      cerr << "for `" << key << "'" << endl;
      return FALSE;
    }
  } else if (key == "elim_heuristic") {
    string value ((char*) YAP_AtomName (YAP_AtomOfTerm (YAP_ARG2)));
    if (       value == "min_neighbors") {
      ElimGraph::setEliminationHeuristic (ElimHeuristic::MIN_NEIGHBORS);
    } else if (value == "min_weight") {
      ElimGraph::setEliminationHeuristic (ElimHeuristic::MIN_WEIGHT);
    } else if (value == "min_fill") {
      ElimGraph::setEliminationHeuristic (ElimHeuristic::MIN_FILL);
    } else if (value == "weighted_min_fill") {
      ElimGraph::setEliminationHeuristic (ElimHeuristic::WEIGHTED_MIN_FILL);
    } else {
      cerr << "warning: invalid value `" << value << "' " ;
      cerr << "for `" << key << "'" << endl;
      return FALSE;
    }
  } else if (key == "schedule") {
    string value ((char*) YAP_AtomName (YAP_AtomOfTerm (YAP_ARG2)));
    if (       value == "seq_fixed") {
      BpOptions::schedule = BpOptions::Schedule::SEQ_FIXED;
    } else if (value == "seq_random") {
      BpOptions::schedule = BpOptions::Schedule::SEQ_RANDOM;
    } else if (value == "parallel") {
      BpOptions::schedule = BpOptions::Schedule::PARALLEL;
    } else if (value == "max_residual") {
      BpOptions::schedule = BpOptions::Schedule::MAX_RESIDUAL;
    } else {
      cerr << "warning: invalid value `" << value << "' " ;
      cerr << "for `" << key << "'" << endl;
      return FALSE;
    }
  } else if (key == "accuracy") {
    BpOptions::accuracy = (double) YAP_FloatOfTerm (YAP_ARG2);
  } else if (key == "max_iter") {
    BpOptions::maxIter = (int) YAP_IntOfTerm (YAP_ARG2);
  } else if (key == "use_logarithms") {
    string value ((char*) YAP_AtomName (YAP_AtomOfTerm (YAP_ARG2)));
    if (       value == "true") {
      Globals::logDomain = true;
    } else if (value == "false") {
      Globals::logDomain = false;
    } else {
      cerr << "warning: invalid value `" << value << "' " ;
      cerr << "for `" << key << "'" << endl;
      return FALSE;
    }
  } else if (key == "order_factor_variables") {
    string value ((char*) YAP_AtomName (YAP_AtomOfTerm (YAP_ARG2)));
    if (       value == "true") {
      FactorGraph::orderFactorVariables = true;
    } else if (value == "false") {
      FactorGraph::orderFactorVariables = false;
    } else {
      cerr << "warning: invalid value `" << value << "' " ;
      cerr << "for `" << key << "'" << endl;
      return FALSE;
    }
  } else {
    cerr << "warning: invalid key `" << key << "'" << endl;
    return FALSE;
  }
  return TRUE;
}



int
freeGroundNetwork (void)
{
  delete (FactorGraph*) YAP_IntOfTerm (YAP_ARG1);
  return TRUE;
}



int
freeParfactors (void)
{
  LiftedNetwork* network = (LiftedNetwork*) YAP_IntOfTerm (YAP_ARG1);
  delete network->first;
  delete network->second;
  delete network;
  return TRUE;
}



extern "C" void
init_predicates (void)
{
  YAP_UserCPredicate ("create_lifted_network", createLiftedNetwork, 3);
  YAP_UserCPredicate ("create_ground_network", createGroundNetwork, 4);
  YAP_UserCPredicate ("run_lifted_solver",     runLiftedSolver,     3);
  YAP_UserCPredicate ("run_ground_solver",     runGroundSolver,     3);
  YAP_UserCPredicate ("set_parfactors_params", setParfactorsParams, 2);
  YAP_UserCPredicate ("set_factors_params",    setFactorsParams,    2);
  YAP_UserCPredicate ("set_vars_information",  setVarsInformation,  2);
  YAP_UserCPredicate ("set_horus_flag",        setHorusFlag,        2);
  YAP_UserCPredicate ("free_parfactors",       freeParfactors,      1);
  YAP_UserCPredicate ("free_ground_network",   freeGroundNetwork,   1);
}

