#ifndef HORUS_PARFACTORLIST_H
#define HORUS_PARFACTORLIST_H

#include <list>
#include <queue>

#include "Parfactor.h"
#include "ProbFormula.h"


using namespace std;


class ParfactorList
{
  public:
    ParfactorList (void) { }

    ParfactorList (const ParfactorList&);

    ParfactorList (const Parfactors&);

   ~ParfactorList (void);

    const list<Parfactor*>& parfactors (void) const { return pfList_; }

    void clear (void) { pfList_.clear(); }

    unsigned size (void) const { return pfList_.size(); }

    typedef std::list<Parfactor*>::iterator iterator;

    iterator begin (void) { return pfList_.begin(); }

    iterator end (void) { return pfList_.end(); }

    typedef std::list<Parfactor*>::const_iterator const_iterator;

    const_iterator begin (void) const { return pfList_.begin(); }

    const_iterator end (void) const { return pfList_.end(); }

    void add (Parfactor* pf);

    void add (const Parfactors& pfs);

    void addShattered (Parfactor* pf);

    list<Parfactor*>::iterator insertShattered (
        list<Parfactor*>::iterator, Parfactor*);

    list<Parfactor*>::iterator remove (list<Parfactor*>::iterator);

    list<Parfactor*>::iterator removeAndDelete (list<Parfactor*>::iterator);

    bool isAllShattered (void) const;

    void print (void) const;

  private:
    
    bool isShattered (const Parfactor*, const Parfactor*) const;

    void addToShatteredList (Parfactor*);
    
    std::pair<Parfactors, Parfactors> shatter (
        Parfactor*, Parfactor*);

    std::pair<Parfactors, Parfactors> shatter (
        unsigned, Parfactor*, unsigned, Parfactor*);

    Parfactors shatter (
        Parfactor*,
        unsigned,
        ConstraintTree*,
        ConstraintTree*,
        unsigned);

    void unifyGroups (unsigned group1, unsigned group2);

    bool proper (
        const ProbFormula&, ConstraintTree,
        const ProbFormula&, ConstraintTree) const;

    bool identical (
        const ProbFormula&, ConstraintTree,
        const ProbFormula&, ConstraintTree) const;

    bool disjoint (
        const ProbFormula&, ConstraintTree,
        const ProbFormula&, ConstraintTree) const;

    list<Parfactor*>  pfList_;
};

#endif // HORUS_PARFACTORLIST_H

