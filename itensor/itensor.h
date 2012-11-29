//
// Distributed under the ITensor Library License, Version 1.0.
//    (See accompanying LICENSE file.)
//
#ifndef __ITENSOR_ITENSOR_H
#define __ITENSOR_ITENSOR_H
#include "real.h"
#include "index.h"
#include "prodstats.h"
#include "indexset.h"

#define ITENSOR_USE_ALLOCATOR

#ifdef ITENSOR_USE_ALLOCATOR
#include "allocator.h"
#endif

//Forward declarations
struct ProductProps;
class Counter;
class Combiner;
class ITDat;
class ITSparse;

//
// ITensor
//
class ITensor
    {
    public:

    //Accessor Methods ----------------------------------------------

    //Real number that uniquely identifies this
    //ITensor's set of Indices (independent of their order)
    Real 
    uniqueReal() const { return is_.ur_; } 

    //Get the jth Index of this ITensor, j = 1,2,..,r()
    const Index& 
    index(int j) const { return is_.index(j); }

    //Rank of this ITensor (number of indices)
    int 
    r() const { return is_.r_; }

    //Number of m!=1 indices
    int 
    rn() const { return is_.rn_; }

    //Bond dimension of jth Index, j = 1,2,..,r()
    int 
    m(int j) const { return is_.m(j); }

    //true if ITensor is default constructed
    bool 
    isNull() const { return (p == 0); }

    //true if ITensor is NOT default constructed
    bool 
    isNotNull() const { return (p != 0); }

    bool 
    isComplex() const { return hasindexn(Index::IndReIm()); }
    bool 
    isNotComplex() const { return !hasindexn(Index::IndReIm()); }

    //Read-only access to scale factor, used internally for efficient scalar ops
    const LogNumber&
    scale() const { return scale_; }

    //Enables looping over Indices in a Foreach
    //e.g. Foreach(const Index& I, t.index() ) { ... }
    const std::pair<IndexSet::index_it,IndexSet::index_it> 
    index() const  
        { return is_.index(); }


    //Constructors --------------------------------------------------

    //Construct Null ITensor, isNull returns true
    ITensor();

    //Construct rank 0 ITensor (scalar), value set to val
    explicit
    ITensor(Real val);

    //Construct rank 1 ITensor, all entries set to zero
    explicit 
    ITensor(const Index& i1);

    //Construct rank 1 ITensor, all entries set to val
    ITensor(const Index& i1, Real val);

    //Construct rank 1 ITensor, entries set to those of V
    ITensor(const Index& i1, const VectorRef& V);

    //Construct rank 2 ITensor, all entries set to zero
    ITensor(const Index& i1,const Index& i2);

    //Construct rank 2 ITensor (a matrix) with 'a' on the diagonal
    ITensor(const Index& i1, const Index& i2, Real a);

    //Construct rank 2 ITensor, entries set to those of M
    ITensor(const Index& i1,const Index& i2,const MatrixRef& M);

    //Construct ITensor up to rank 8, entries set to zero
    ITensor(const Index& i1, const Index& i2, const Index& i3,
            const Index& i4 = Index::Null(), 
            const Index& i5 = Index::Null(), 
            const Index& i6 = Index::Null(),
            const Index& i7 = Index::Null(), 
            const Index& i8 = Index::Null());

    // Construct rank 1 tensor T from IndexVal iv = (I,n)
    // (I is an Index, n an int)
    // such that T(I(n)) == val
    explicit 
    ITensor(const IndexVal& iv, Real val = 1);

    // Construct rank 2 tensor T from IndexVals 
    // iv1 = (I1,n1), iv2 = (I2,n2)
    // such that T(I1(n1),I2(n2)) == 1
    ITensor(const IndexVal& iv1, const IndexVal& iv2);

    // Construct tensor T from up to 8 IndexVals 
    // iv1 = (I1,n1), iv2 = (I2,n2), iv3 = (I3,n3), ...
    // such that T(I1(n1),I2(n2),I3(n3),...) == 1
    ITensor(const IndexVal& iv1, const IndexVal& iv2, 
            const IndexVal& iv3, const IndexVal& iv4 = IndexVal::Null(), 
            const IndexVal& iv5 = IndexVal::Null(), const IndexVal& iv6 = IndexVal::Null(), 
            const IndexVal& iv7 = IndexVal::Null(), const IndexVal& iv8 = IndexVal::Null());

    explicit 
    ITensor(const std::vector<Index>& I);

    ITensor(const std::vector<Index>& I, const Vector& V);

    ITensor(const std::vector<Index>& I, const ITensor& other);

    ITensor(const std::vector<Index>& I, const ITensor& other, Permutation P);

    explicit
    ITensor(std::istream& s) { read(s); }

    static const ITensor& 
    Complex_1();

    static const ITensor& 
    Complex_i();

    static const ITensor& 
    ConjTensor();
        
    //Read in ITensor from binary stream s
    void 
    read(std::istream& s);

    //Write out ITensor to binary stream s
    void
    write(std::ostream& s) const;


    //
    // Operators
    //

    //Contracting product
    //All matching Index pairs automatically contracted
    //Cji = \sum_{k,l} Akjl * Blki
    ITensor& 
    operator*=(const ITensor& other);

    ITensor 
    operator*(const ITensor& other) const 
        { ITensor res(*this); res *= other; return res; }

    // Contract with IndexVal
    // If iv = (J,n), Index J is fixed to it's nth
    // value and rank decreases by 1
    // (similar to summing against a Kronecker
    // delta tensor \delta_{J,n})
    ITensor& 
    operator*=(const IndexVal& iv) 
        { return operator*=(ITensor(iv)); } 

    ITensor 
    operator*(const IndexVal& iv) const 
        { ITensor res(*this); res *= iv; return res; }

    friend inline ITensor 
    operator*(const IndexVal& iv, const ITensor& t) 
        { return (ITensor(iv) *= t); }

    //Multiplication and division by scalar
    ITensor& 
    operator*=(Real fac) { scale_ *= fac; return *this; }

    ITensor 
    operator*(Real fac) const 
        { ITensor res(*this); res *= fac; return res; }

    friend inline ITensor 
    operator*(Real fac, ITensor t) 
        { return (t *= fac); }

    ITensor& 
    operator/=(Real fac) { scale_ /= fac; return *this; }

    ITensor 
    operator/(Real fac) const 
        { ITensor res(*this); res /= fac; return res; }

    friend inline ITensor 
    operator/(Real fac, ITensor t) 
        { return (t /= fac); }

    //Multiplication with LogNumber (very large or very small Real)
    ITensor& 
    operator*=(const LogNumber& lgnum) { scale_ *= lgnum; return *this; }

    ITensor 
    operator*(const LogNumber& lgnum) const 
        { ITensor res(*this); res *= lgnum; return res; }

    friend inline ITensor 
    operator*(const LogNumber& lgnum, ITensor t) 
        { return (t *= lgnum); }


    //Non-contracting product
    //Matching Index pairs are merged
    //Ckjli = Akjl * Blki
    ITensor& 
    operator/=(const ITensor& other);

    ITensor 
    operator/(const ITensor& other) const 
        { ITensor res(*this); res /= other; return res; }

    //Tensor addition and subtraction
    //Summands must have same Indices, in any order
    //Cijk = Aijk + Bkij
    ITensor& 
    operator+=(const ITensor& o);

    ITensor 
    operator+(const ITensor& o) const 
        { ITensor res(*this); res += o; return res; }

    ITensor& 
    operator-=(const ITensor& o)
        {
        if(this == &o) { scale_ = 0; return *this; }
        scale_ *= -1; operator+=(o); scale_ *= -1; return *this; 
        }

    ITensor 
    operator-(const ITensor& o) const 
        { ITensor res(*this); res -= o; return res; }


    //Index Methods ---------------------------------------------------

    //Return first Index of matching type
    const Index&
    findtype(IndexType t) const { return is_.findtype(t); }

    //Return position of matching Index, 0 if not found
    int 
    findindex(const Index& I) const { return is_.findindex(I); }

    //Return position of m!=1 Index, 0 if not found
    int 
    findindexn(const Index& I) const { return is_.findindexn(I); }

    //Return position of m==1 Index, 0 if not found
    int 
    findindex1(const Index& I) const { return is_.findindex1(I); }

    //true if other has any Index in common with this
    bool 
    has_common_index(const ITensor& other) const
        { return is_.has_common_index(other.is_); }
    
    //true if has Index I
    bool 
    hasindex(const Index& I) const { return is_.hasindex(I); }

    //true if has m!=1 Index I
    bool 
    hasindexn(const Index& I) const { return is_.hasindexn(I); }

    //true if has m==1 Index I
    bool 
    hasindex1(const Index& I) const { return is_.hasindex1(I); }

    //true if this tensor has the first nind Indices in array I
    bool
    hasAllIndex(const boost::array<Index,NMAX+1>& I, int nind) const
        { return is_.hasAllIndex(I,nind); }

    //Add m==1 Index I to this, increasing rank by 1
    void 
    addindex1(const Index& I) { is_.addindex1(I); }

    //Add all m==1 Indices in indices to this
    void 
    addindex1(const std::vector<Index>& indices) { is_.addindex1(indices); }

    //Remove jth m==1 index as found by findindex
    void 
    removeindex1(int j) { is_.removeindex1(j); }

    //Remove m==1 Index matching I
    void 
    removeindex1(const Index& I) { is_.removeindex1(is_.findindex1(I)); }

    //Replace Index i1 with Index i2, throws ITError if i1.m() != i2.m()
    void 
    mapindex(const Index& i1, const Index& i2) { is_.mapindex(i1,i2); }

    //Primelevel Methods ------------------------------------

    //Set primeLevel of Indices to zero
    void 
    noprime(IndexType type = All) { is_.noprime(type); }

    //Increase primeLevel of Indices by 1 (or optional amount inc)
    void 
    doprime(IndexType type, int inc = 1) { is_.doprime(type,inc); }

    //Increase primeLevel of all Indices by 1 (or optional amount inc)
    void 
    primeall() { doprime(All,1); }

    //Increase primeLevel of all Site Indices by 1 (or optional amount inc)
    void 
    primesite(int inc = 1) { doprime(Site,inc); }

    //Increase primeLevel of all Link Indices by 1 (or optional amount inc)
    void 
    primelink(int inc = 1) { doprime(Link,inc); }

    //Change all Indices having primeLevel plevold to have primeLevel plevnew
    void 
    mapprime(int plevold, int plevnew, IndexType type = All)
        { is_.mapprime(plevold,plevnew,type); }

    //Change primeLevel of Index I from plevold to plevnew
    //If I.primeLevel() != plevold, has no effect
    void 
    mapprimeind(const Index& I, int plevold, int plevnew, 
                IndexType type = All)
        { is_.mapprimeind(I,plevold,plevnew,type); }

    //Increase primeLevel of Index I by 1 (or optional amount inc)
    void 
    primeind(const Index& I, int inc = 1)
        { mapindex(I,primed(I,inc)); }

    //Increase primeLevel of Index I and Index J by 1
    void 
    primeind(const Index& I, const Index& J) { is_.primeind(I,J); }

    //Set primeLevel of Index I to zero
    void 
    noprimeind(const Index& I) { mapindex(I,I.deprimed()); }

    //Return copy of ITensor with primeLevel of all Indices increased by 1
    ITensor friend inline
    primed(ITensor A, int inc = 1)
        { A.doprime(All,inc); return A; }

    //Return copy of ITensor with primeLevel of all Site Indices increased by 1
    ITensor friend inline
    primesite(ITensor A, int inc = 1)
        { A.doprime(Site,inc); return A; }

    //Return copy of ITensor with primeLevel of all Link Indices increased by 1
    ITensor friend inline
    primelink(ITensor A, int inc = 1)
        { A.doprime(Link,inc); return A; }

    //Return copy of ITensor with primeLevel of Index I increased by 1
    //(or optional amount inc)
    ITensor friend inline
    primeind(ITensor A, const Index& I, int inc = 1)
        { A.mapindex(I,primed(I,inc)); return A; }

    //Return copy of ITensor with primeLevel of Index I1 and I2 increased by 1
    ITensor friend
    primeind(ITensor A, const Index& I1, const Index& I2);

    //Return copy of ITensor with primeLevel of all Indices set to zero
    ITensor friend inline
    deprimed(ITensor A) { A.noprime(); return A; }

    //Element Access Methods ----------------------------------------

    //Get scalar value of rank 0 ITensor
    //Throws ITError if r() != 0
    Real
    toReal() const { return val0(); }

    //Get scalar value of rank 0 ITensor
    //Throws ITError if r() != 0
    Real 
    val0() const;

    //Get element j of rank 1 ITensor
    //Throws ITError if rn() != 1
    Real 
    val1(int i1) const;

    // IndexVal element access
    // Given iv1 = (I1,n1), iv2 = (I2,n2), ...
    // returns component of ITensor such that
    // I1 temporarily set to n1, I2 to n2, etc.
    // Can be used to set components of ITensors
    // as well, for example, T(I1(2),I2(1)) = 3;
    Real& 
    operator()();

    Real 
    operator()() const;

    Real& 
    operator()(const IndexVal& iv1);

    Real operator()(const IndexVal& iv1) const;

    Real& 
    operator()(const IndexVal& iv1, const IndexVal& iv2);

    Real 
    operator()(const IndexVal& iv1, const IndexVal& iv2) const;

    Real& 
    operator()(const IndexVal& iv1, const IndexVal& iv2, 
               const IndexVal& iv3, const IndexVal& iv4 = IndexVal::Null(), 
               const IndexVal& iv5 = IndexVal::Null(),const IndexVal& iv6 = IndexVal::Null(),
               const IndexVal& iv7 = IndexVal::Null(),const IndexVal& iv8 = IndexVal::Null());

    Real 
    operator()(const IndexVal& iv1, const IndexVal& iv2, 
               const IndexVal& iv3, const IndexVal& iv4 = IndexVal::Null(), 
               const IndexVal& iv5 = IndexVal::Null(),const IndexVal& iv6 = IndexVal::Null(),
               const IndexVal& iv7 = IndexVal::Null(),const IndexVal& iv8 = IndexVal::Null()) 
    const;


    //Methods for Mapping to Other Objects ----------------------------------

    //
    // Assume *this and other have same indices but different order.
    // Copy other into *this, without changing the order of indices in either.
    // Assignment operator= would put the order of other into *this.
    //
    void 
    assignFrom(const ITensor& other);

    //
    // groupIndices combines a set of indices (of possibly different sizes) 
    // together, leaving only single grouped Index.
    //
    // RiJ = Ai(jk) <-- Here J represents the grouped pair of indices (jk)
    //                  If j.m() == 5 and k.m() == 7, J.m() == 5*7.
    //
    void 
    groupIndices(const boost::array<Index,NMAX+1>& indices, int nind, 
                      const Index& grouped, ITensor& res) const;

    //
    // tieIndices locks a set of indices (of the same size) together,
    // leaving only a single tied Index.
    //
    // Rijl = Aijil <-- here we have tied the 1st and 3rd index of A
    //
    void
    tieIndices(const boost::array<Index,NMAX+1>& indices, int nind,
               const Index& tied);

    void
    tieIndices(const Index& i1, const Index& i2,
               const Index& tied);

    friend inline ITensor
    tieIndices(const Index& i1, const Index& i2, 
               const Index& tied, ITensor T)
        { T.tieIndices(i1,i2,tied); return T; }

    void
    tieIndices(const Index& i1, const Index& i2,
               const Index& i3,
               const Index& tied);

    friend inline ITensor
    tieIndices(const Index& i1, const Index& i2, 
               const Index& i3, const Index& tied, ITensor T)
        { T.tieIndices(i1,i2,i3,tied); return T; }

    void
    tieIndices(const Index& i1, const Index& i2,
               const Index& i3, const Index& i4,
               const Index& tied);

    friend inline ITensor
    tieIndices(const Index& i1, const Index& i2, 
               const Index& i3, const Index& i4, 
               const Index& tied, ITensor T)
        { T.tieIndices(i1,i2,i3,i4,tied); return T; }

    //
    // The trace method sums over the given set of indices
    // (which must all have the same dimension).
    //
    // Rik = trace(j,l,m,Aijkml) = \sum_j Aijkjj
    //

    void
    trace(const boost::array<Index,NMAX+1>& indices, int nind);

    void
    trace(const Index& i1, const Index& i2);

    void
    trace(const Index& i1);

    ITensor friend inline
    trace(const Index& i1, const Index& i2, ITensor T)
        { T.trace(i1,i2); return T; }

    void
    trace(const Index& i1, const Index& i2, const Index& i3);

    ITensor friend inline
    trace(const Index& i1, const Index& i2, const Index& i3,
          ITensor T)
        { T.trace(i1,i2,i3); return T; }

    void
    trace(const Index& i1, const Index& i2, const Index& i3, const Index& i4);

    ITensor friend inline
    trace(const Index& i1, const Index& i2, const Index& i3, const Index& i4,
          ITensor T)
        { T.trace(i1,i2,i3,i4); return T; }

    //
    // Tracing over all indices results in a Real
    //
    Real friend inline
    trace(ITensor T)
        {
        if(T.rn() != 0) T.trace(T.is_.index_,T.rn());
        return T.val0();
        }

    //
    // expandIndex replaces a smaller index with a bigger one, padding out
    // the elements of the resulting ITensor with zeros as necessary.
    // Say we have a tensor Aij and j has range m. Now expand j with 
    // a larger Index J. The result is RiJ, where
    //        _
    // RiJ = |  Ai(j=J-start+1) for J = start...start+m
    //       |_ 0               otherwise
    //        
    void 
    expandIndex(const Index& small, const Index& big, int start);

    //Set components of rank 2 ITensor using Matrix M as input
    void 
    fromMatrix11(const Index& i1, const Index& i2, const Matrix& M);

    //Convert rank 2 ITensor to a Matrix using given Index order
    void 
    toMatrix11(const Index& i1, const Index& i2, Matrix& res) const;

    //Convert rank 2 ITensor to a Matrix, but do not include
    //scale factor in result
    void 
    toMatrix11NoScale(const Index& i1, const Index& i2, 
                           Matrix& res) const;

    // group i1,i2; i3,i4
    void toMatrix22(const Index& i1, const Index& i2, 
                    const Index& i3, const Index& i4, Matrix& res) const;
    /*
    void fromMatrix22(const Index& i1, const Index& i2, 
                      const Index& i3, const Index& i4,const Matrix& res);

    // group i1,i2; i3
    void toMatrix21(const Index& i1, const Index& i2, 
                    const Index& i3, Matrix& res) const;
    void fromMatrix21(const Index& i1, const Index& i2, 
                      const Index& i3, const Matrix& res);

    */

    // group i1; i2,i3
    void toMatrix12NoScale(const Index& i1, const Index& i2, 
                           const Index& i3, Matrix& res) const;

    void toMatrix12(const Index& i1, const Index& i2, 
                    const Index& i3, Matrix& res) const;

    void fromMatrix12(const Index& i1, const Index& i2, 
                      const Index& i3, const Matrix& M);

    void
    symmetricDiag11(const Index& i1, ITensor& D, ITensor& U, Index& mid) const;

    void
    symmetricDiag11(const Index& i1, ITensor& D, ITensor& U, Index& mid, int& mink, int& maxk) const;

    int 
    vecSize() const;

    int 
    maxSize() const;

    void 
    assignToVec(VectorRef v) const;

    void 
    assignFromVec(const VectorRef& v);

    void 
    reshapeDat(const Permutation& p, Vector& rdat) const;

    //In-place version of reshapeDat. Does not re-order indices
    //so resulting ITensor is *not* equivalent to original.
    void 
    reshapeDat(const Permutation& P);

    //Re-orders indices and dat consistently
    //so resulting ITensor *is* equivalent to original.
    void 
    reshape(const Permutation& P) const;

    void 
    reshapeTo(const Permutation& P, ITensor& res) const;


    //
    // Swap can be used for similar purposes
    // as operator=(const ITensor& other)
    // but is more efficient and has same
    // end result if other is just a temporary
    //
    void
    swap(ITensor& other);


    //Other Methods -------------------------------------------------

    void 
    Randomize();

    void 
    SplitReIm(ITensor& re, ITensor& im) const;

    void 
    conj() 
        { 
        if(!isComplex()) return; 
        operator/=(ITensor::ConjTensor()); 
        }

    void 
    conj(const Index& I) { }

    //bool 			// Why not norm() == 0.0?  
    //is_zero() const { return (norm() < 1E-20); } 

    Real 
    sumels() const;

    Real 
    norm() const;

    template <typename Callable> void
    mapElems(const Callable& f);

    void
    pseudoInvert(Real cutoff = 0);

    void 
    scaleOutNorm() const;

    void 
    scaleTo(LogNumber newscale) const;

    void 
    print(std::string name = "",Printdat pdat = HideData) const;

    void 
    printIndices(const std::string& name = "") const
        { print(name,HideData); }

    void 
    printIndices(const boost::format& fname) const
        { printIndices(fname.str()); }

    friend std::ostream& 
    operator<<(std::ostream & s, const ITensor & t);

    friend class commaInit;

    typedef Index 
    IndexT;

    typedef IndexVal 
    IndexValT;

    typedef Combiner 
    CombinerT;

    typedef ITSparse
    SparseT;

    static const Index& 
    ReImIndex() { return Index::IndReIm(); }

    protected:

    //////////////
    //
    // Data Members
    //

    //mutable: const methods may want to reshape data
    mutable boost::intrusive_ptr<ITDat> p; 

    //Indices, maximum of 8 (is_.index_[0] not used)
    mutable IndexSet is_;

    //mutable since e.g. scaleTo is logically const
    mutable LogNumber scale_; 

    //
    //
    //////////////

    void 
    initCounter(Counter& C) const;

    void 
    allocate(int dim);

    void 
    allocate();

    //Disattach self from current ITDat and create own copy instead.
    //Necessary because ITensors logically represent distinct
    //objects even though they may share data in reality.
    void 
    solo() const;
    
    friend struct ProductProps;

    friend void toMatrixProd(const ITensor& L, const ITensor& R, 
                             ProductProps& pp,
                             MatrixRefNoLink& lref, MatrixRefNoLink& rref,
                             bool& L_is_matrix, bool& R_is_matrix);

    void
    directMultiply(const ITensor& other, ProductProps& pp, 
                   int& new_rn_, boost::array<Index,NMAX+1>& new_index_);

    int _ind(int i1, int i2, int i3, int i4, 
             int i5, int i6, int i7, int i8) const;

    int _ind2(const IndexVal& iv1, const IndexVal& iv2) const;

    int _ind8(const IndexVal& iv1, const IndexVal& iv2, 
              const IndexVal& iv3, const IndexVal& iv4 = IndexVal::Null(), 
              const IndexVal& iv5 = IndexVal::Null(),const IndexVal& iv6 = IndexVal::Null(),
              const IndexVal& iv7 = IndexVal::Null(),const IndexVal& iv8 = IndexVal::Null())
        const;

    friend class ITSparse;

    friend void 
    product(const ITSparse& S, const ITensor& T, ITensor& res);

    public:

    // The ITmaker constructor is for making constant, global
    // ITensors and is not intended to be called by users,
    // only internally by static ITensor methods
    enum ITmaker { makeComplex_1, makeComplex_i, makeConjTensor };

    ITensor(ITmaker itm);

    }; // class ITensor

//
// Counter
//
class Counter
    {
public:
    boost::array<int,NMAX+1> n, i;
    int ind;
    int rn_,r_;

    Counter();

    Counter(const boost::array<Index,NMAX+1>& ii,int rn,int r);

    Counter(const IndexSet& is);

    void 
    init(const boost::array<Index,NMAX+1>& ii, int rn, int r);

    void 
    init(const IndexSet& is);

    Counter& 
    operator++();

    bool 
    operator!=(const Counter& other) const;

    bool 
    operator==(const Counter& other) const;

    bool 
    notDone() const 
        { return i[1] != 0; }

    friend std::ostream& 
    operator<<(std::ostream& s, const Counter& c);

    void 
    reset(int a);

    };

//
// ITDat
//
class ITDat
    {
public:

    Vector v;

    ITDat();

    explicit 
    ITDat(int size);

    explicit 
    ITDat(const VectorRef& v_);

    explicit 
    ITDat(Real r);

    explicit 
    ITDat(std::istream& s);

    explicit 
    ITDat(const ITDat& other);

    void 
    write(std::ostream& s) const;
    
    void 
    print() const 
        { std::cout << "ITDat: v = " << v; }


#ifdef ITENSOR_USE_ALLOCATOR
    void* operator 
    new(size_t) throw(std::bad_alloc)
        { return allocator().alloc(); }

    void operator 
    delete(void* p) throw()
        { return allocator().dealloc(p); }

    static DatAllocator<ITDat>& allocator()
        {
        static DatAllocator<ITDat> allocator_;
        return allocator_;
        }
#endif

    friend class ITensor;

    friend void 
    intrusive_ptr_add_ref(ITDat* p);

    friend void 
    intrusive_ptr_release(ITDat* p);

    int count() const { return numref; }

private:

    mutable unsigned int 
    numref;

    //Must be dynamically allocated:
    void operator=(const ITDat&);
    ~ITDat() { }


    };


class commaInit
    {
public:
    commaInit(ITensor& T_)
        : T(T_)
        { 
        if(T.isNull()) Error("Can't assign to null ITensor");
        T.solo();
        T.scaleTo(1);
        T.initCounter(c);
        }

    commaInit& operator<<(Real r)
        {
        return operator,(r);
        }

    commaInit& operator,(Real r)
        {
        if(c.notDone()) 
            { T.p->v(c.ind) = r; ++c; }
        else 
            { Error("Comma assignment list too long.\n"); }
        return *this;
        }

private:
    ITensor& T;
    Counter c; 
    };

template <typename Callable> void ITensor::
mapElems(const Callable& f)
    {
    solo();
    scaleTo(1);
    for(int j = 1; j <= p->v.Length(); ++j)
        p->v(j) = f(p->v(j));
    }

//
// Computes the scalar/inner/dot product of two
// real-valued ITensors.
//
// Equivalent to the ITensor contraction x * y 
// except the result is a Real number instead
// of a rank 0 ITensor.
//
Real 
Dot(const ITensor& x, const ITensor& y);

//
// Computes the scalar (inner) product of two
// possibly complex ITensors.
//
// The first argument gets conjugated so this method
// is equivalent to the ITensor contraction conj(x) * y 
// except the result is two real numbers (re and im) 
// instead of a rank 0 ITensor.
//
void 
BraKet(const ITensor& x, const ITensor& y, Real& re, Real& im);

//
// Define product of IndexVal iv1 = (I1,n1), iv2 = (I2,n2)
// (I1, I2 are Index objects; n1,n2 are type int)
// to be an ITensor T such that T(I1(n1),I2(n2)) == 1
//
// Useful for creating MPOs
//
ITensor inline
operator*(const IndexVal& iv1, const IndexVal& iv2) 
    { ITensor t(iv1); return (t *= iv2); }

//
// Define product of IndexVal iv1 = (I1,n1) with a Real "val"
// to be an ITensor T such that T(I1(n1)) == val
//
// Useful for creating MPOs
//
ITensor inline
operator*(const IndexVal& iv1, Real val) 
    { return ITensor(iv1,val); }

ITensor inline
operator*(Real fac, const IndexVal& iv) 
    { return ITensor(iv,fac); }


//
// Given Tensors which represent operator matrices
// (e.g. A(site1',site1), B(site1',site1) )
// multiply them, automatically adjusting primeLevels
// so that result is again an operator matrix C(site1',site1)
//
template<class Tensor>
Tensor inline
multSiteOps(Tensor A, const Tensor& B) 
    {
    A.mapprime(1,2,Site);
    A.mapprime(0,1,Site);
    A *= B;
    A.mapprime(2,1,Site);
    return A;
    }

#endif
