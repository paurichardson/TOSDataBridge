/* 
Copyright (C) 2014 Jonathon Ogden     < jeog.dev@gmail.com >

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see http://www.gnu.org/licenses.
*/

#ifndef JO_TOSDB_DATA_STREAM
#define JO_TOSDB_DATA_STREAM

#ifndef STR_DATA_SZ
#define STR_DATA_SZ ((unsigned long)0xFF)
#endif

#include <deque>
#include <string>
#include <vector>
#include <mutex>

/*    
    tosdb_data_stream is interfaced differently than the typical templatized cotainer.
    Since it provides its own java-style interface, and exceptions, it should also 
    provide a non-templatized namespace for access to generic, static 'stuff'
    while making it clear that an interface is being used/expected.
    
    The interface is of type ::Interface<type,type>
    The container is of type ::Object<type,type,type,bool,type> 

    We've hard-coded a max bound size as INT_MAX to avoid some of the corner cases.
*/

namespace tosdb_data_stream {

class error : public std::exception{
public:
    error( const char* info ) 
        : 
        std::exception( info ) 
        {
        }
};
    class type_error : public error{
    public:
        type_error( const char* info ) 
            : 
            error( info ) 
            { 
            }
    };
    class size_violation : public error{
    public:
        const size_t boundSize, dequeSize;            
        size_violation( const char* msg, size_t bSz, size_t dSz )
            : 
            error( msg ),
            boundSize( bSz ),
            dequeSize( dSz )                
            {                
            }
    };
    /* can't make std::exception a virtual base from the 
        stdexcept path so double construction of std::exception */
    class out_of_range : 
        public error, 
        public std::out_of_range {            
    public:
        const int size, beg, end;
        out_of_range( const char* msg, int sz, int beg, int end )
            : 
            std::out_of_range( msg ), 
            error( msg ),                            
            size( sz ), 
            beg( beg ), 
            end( end )
            {                
            }
        virtual const char* what() const 
        { 
            return error::what(); 
        }        
    };
    class invalid_argument : 
        public error, 
        public std::invalid_argument {
    public:    
        invalid_argument( const char* msg )
            : 
            std::invalid_argument( msg ), 
            error( msg )
            {                
            }
        virtual const char* what() const 
        {
            return error::what();
        }
    };
    
/*    The interface to tosdb_data_stream     */
template< typename SecTy, typename GenTy >
class Interface {

public:
    typedef GenTy                     generic_type;
    typedef SecTy                     secondary_type;
    typedef std::pair< GenTy, SecTy>  both_type;
    typedef std::vector< GenTy >      generic_vector_type;
    typedef std::vector< SecTy >      secondary_vector_type;

    static const size_t MAX_BOUND_SIZE = INT_MAX;

private:
    template < typename _inTy >
    void _throw_type_error( const char* method, bool fThru = false ) const
    {         
        std::ostringstream msgStrm;  
        msgStrm << "tosdb_data_stream: Invalid argument < "  
                << ( fThru ? "UNKNOWN" : typeid(_inTy).name() ) 
                << " > passed to method " << method 
                <<" for this template instantiation.";
        throw type_error( msgStrm.str().c_str() );        
    }

    template < typename _inTy, typename _outTy >
    void _copy( _outTy* dest, size_t sz, int end, int beg, secondary_type* sec ) const
    {    
        if( !dest )
            throw invalid_argument( " ->copy(): *dest argument can not be null ");         
        std::unique_ptr< _inTy, void(*)(_inTy*)> 
            tmp( new _inTy[sz], [](_inTy* _ptr){ delete[] _ptr; } );
        copy( tmp.get(), sz, end, beg, sec );
        for( size_t i = 0; i < sz; ++i )            
            dest[i] = (_outTy)tmp.get()[i];                                
    }    

protected:
    unsigned short _count1;
    Interface()
        : _count1( 0 )
        { 
        }

public:
    virtual size_t                  bound_size() const = 0;
    virtual size_t                  bound_size( size_t ) = 0;
    virtual size_t                  size() const = 0;
    virtual bool                    empty() const = 0;    
    virtual bool                    uses_secondary() = 0;
    virtual generic_type            operator[]( int ) const    = 0;
    virtual both_type               both( int ) const = 0;    
    virtual generic_vector_type     vector(int end = -1, int beg = 0) const = 0;
    virtual secondary_vector_type   secondary_vector(int end = -1, int beg = 0) const = 0;    
    
    virtual void push( const generic_type& obj, secondary_type&& sec = secondary_type()) = 0;

    virtual void secondary( secondary_type* dest, int indx ) const 
    { 
        dest = nullptr; /* SHOULD WE THROW? */        
    }
    
    /* Avoid constructing GenTy if possible: the following mess provides
       something of a recursive / drop-thru, safe, type-finding mechanism to do that.    
       Throws if it can't reconcile the passed type at runtime( obviously 
       this is not ideal, but seems to be necessary for what's trying to be accomplished ) */
#define virtual_void_push_2arg_DROP( _inTy, _outTy ) \
    virtual void push( const _inTy val, secondary_type&& sec = secondary_type()) \
    { \
        push( (_outTy)val, std::move(sec) ); \
    } 
#define virtual_void_push_2arg_BREAK( _inTy ) \
    virtual void push( const _inTy val, secondary_type&& sec = secondary_type()) \
    { \
        push( std::to_string(val) , std::move(sec) ); \
    } 
#define virtual_void_push_2arg_LOOP( _inTy, _loopOnC1 ) \
    virtual void push( const _inTy str, secondary_type&& sec = secondary_type()) \
    { \
        if( _count1++ ) \
        { \
            _count1 = 0; \
            _throw_type_error< const _inTy >( "->push()", true ); \
        } \
        push( _loopOnC1, std::move(sec) ); \
    } 
    virtual_void_push_2arg_DROP( float, double )
    virtual_void_push_2arg_BREAK( double )
    virtual_void_push_2arg_DROP( unsigned char, unsigned short )
    virtual_void_push_2arg_DROP( unsigned short, unsigned int )
    virtual_void_push_2arg_DROP( unsigned int, unsigned long )
    virtual_void_push_2arg_DROP( unsigned long, unsigned long long )
    virtual_void_push_2arg_BREAK( unsigned long long )
    virtual_void_push_2arg_DROP( char, short )
    virtual_void_push_2arg_DROP( short, int )
    virtual_void_push_2arg_DROP( int, long )
    virtual_void_push_2arg_DROP( long, long long )
    virtual_void_push_2arg_BREAK( long long )
    virtual_void_push_2arg_LOOP( std::string, str.c_str() )
    virtual_void_push_2arg_LOOP( char*, std::string( str ) )

#define virtual_void_copy_2arg_DROP( _inTy, _outTy ) \
    virtual void copy( _inTy* dest, size_t sz, int end = -1, int beg = 0, secondary_type* sec = nullptr) const \
    { \
        _copy< _outTy >( dest, sz, end, beg, sec ); \
    } 
#define virtual_void_copy_2arg_BREAK( _inTy, _dropBool ) \
    virtual void copy( _inTy* dest, size_t sz, int end = -1, int beg = 0, secondary_type* sec = nullptr) const \
    { \
        _throw_type_error< _inTy* >( "->copy()", _dropBool ); \
    }

    virtual_void_copy_2arg_DROP( long long, long )
    virtual_void_copy_2arg_DROP( long, int )
    virtual_void_copy_2arg_DROP( int, short )
    virtual_void_copy_2arg_DROP( short, char )
    virtual_void_copy_2arg_BREAK( char, true )
    virtual_void_copy_2arg_DROP( unsigned long long, unsigned long )
    virtual_void_copy_2arg_DROP( unsigned long, unsigned int )
    virtual_void_copy_2arg_DROP( unsigned int, unsigned short )
    virtual_void_copy_2arg_DROP( unsigned short, unsigned char )
    virtual_void_copy_2arg_BREAK( unsigned char, true )
    virtual_void_copy_2arg_DROP( double, float )
    virtual_void_copy_2arg_BREAK( float, false ) 

    virtual void 
    copy( char** dest, size_t destSz, size_t strSz, int end = -1, int beg = 0 , secondary_type* sec = nullptr ) const 
    { 
        _throw_type_error< std::string* >( "->copy()", false );         
    }

    virtual void 
    copy( std::string* dest, size_t sz, int end = -1, int beg = 0, secondary_type* sec = nullptr ) const
    {
        if( !dest )
            throw invalid_argument( " ->copy(): *dest argument can not be null ");
        auto dstr = [sz]( char** _pptr){ DeallocStrArray( _pptr, sz); };
        std::unique_ptr< char*, decltype( dstr ) >
            strMat( AllocStrArray( sz, STR_DATA_SZ ), dstr );
        this->copy( strMat.get(), sz, STR_DATA_SZ , end, beg, sec);                
        std::copy_n( strMat.get(), sz, dest );            
    }
    
    virtual ~Interface()
        {
        }
};

/*    The container object w/o secondary deque */
template < typename Ty,
            typename SecTy,
             typename GenTy,            
              bool UseSecondary = false,
               typename Allocator = std::allocator<Ty> >
class Object
    : public Interface<SecTy, GenTy>{

    class{
        static const bool valid = GenTy::Type_Check<Ty>::value;    
        static_assert( valid, 
            "tosdb_data_stream::object can not be compiled; \
            Ty failed GenTy's type-check;" ); 
    }_inst_check_;    
    
    typedef Object< Ty, SecTy, GenTy, UseSecondary, Allocator>  _myTy;
    typedef Interface< SecTy, GenTy >                           _myBase;
    typedef std::deque< Ty, Allocator >                         _myImplTy;            
    typedef typename _myImplTy::const_iterator::difference_type _myImplDiffTy;    

    _myTy& operator=(const _myTy &);
    
    void _push(const Ty _item) 
    {    /* if can't obtain lock indicate other threads should yield to us */        
        /* using raw locking, push/pop doesn't throw */
        _push_has_priority = _mtx->try_lock();
        if( !_push_has_priority )
            _mtx->lock(); /* block regardless */                 
        _myImplObj.push_front(_item); 
        _myImplObj.pop_back();        
        if( _qCount < _qBound )
            ++_qCount;
        _mtx->unlock();
    } 

protected:
    typedef std::lock_guard<std::recursive_mutex >  _guardTy;
    
    std::recursive_mutex* const  _mtx; 
    volatile bool                _push_has_priority;    
    size_t                       _qCount, _qBound;    
    _myImplTy                    _myImplObj;

    void _yld_to_push() const
    {    
        if( _push_has_priority )
            return;
        std::this_thread::yield();
    }

    template< typename T > bool 
    _check_adj( int& end, int& beg, const std::deque< T, Allocator>& impl ) const
    {  /* since sz can't be > INT_MAX this won't be a problem */
        int sz = (int)impl.size();
        if ( _qBound != sz )    
            throw size_violation( 
                "Internal size/bounds violation in tosdb_data_stream",
                _qBound, sz 
                );            
        if ( end < 0 ) end += sz; 
        if ( beg < 0 ) beg += sz;
        if ( beg >= sz || end >= sz || beg < 0 || end < 0 )    
            throw out_of_range(
                "adjusted index value out of range in tosdb_data_stream",
                sz, beg, end
                );
        else if ( beg > end )     
            throw invalid_argument( 
                "adjusted beginning index value > ending index value in tosdb_data_stream"                 
                );
        return true;
    }

    template< typename ImplTy, typename DestTy > void 
    _copy_to_ptr( ImplTy& impl, DestTy* dest, size_t sz, unsigned int end, unsigned int beg) const
    {    
        ImplTy::const_iterator bIter = impl.cbegin() + beg;
        ImplTy::const_iterator eIter 
            = impl.cbegin() + std::min< size_t >( sz + beg, std::min< size_t >( ++end , _qCount ) );
        if( bIter < eIter )
            std::copy( bIter, eIter, dest );     
    }

public:
    typedef _myBase  interface_type;
    typedef Ty       value_type;

    Object(size_t sz )
        : 
        _myImplObj( std::min<size_t>(sz,MAX_BOUND_SIZE) ),
        _qBound( std::min<size_t>(sz,MAX_BOUND_SIZE) ),
        _qCount( 0 ),
        _mtx( new std::recursive_mutex ),
        _push_has_priority( true )
        {            
        }

    Object(const _myTy & stream )
        : 
        _myImplObj( stream._myImplObj ),
        _qBound( stream._qBound ),
        _qCount( stream._qCount ),
        _mtx( new std::recursive_mutex ),
        _push_has_priority( true )
        {            
        }

    Object( _myTy && stream)
        : 
        _myImplObj( std::move( stream._myImplObj) ),
        _qBound( stream._qBound ),
        _qCount( stream._qCount ),                
        _mtx( new std::recursive_mutex ),
        _push_has_priority( true )
        {                
        }

    virtual ~Object()
        {
            delete this->_mtx;        
        }

    bool empty() const 
    { 
        return _myImplObj.empty(); 
    }

    size_t size() const
    {
        return _qCount;
    }

    size_t bound_size() const
    {
        return _qBound;        
    }

    size_t bound_size( size_t sz )
    {
        sz = std::min<size_t>(sz,MAX_BOUND_SIZE);
        _guardTy _lock_( *_mtx );
        _myImplObj.resize(sz);
        if (sz < _qBound)
            _myImplObj.shrink_to_fit();    
        if( sz < _qCount )
            _qCount = sz;
        return (_qBound = sz);
    }

    bool inline uses_secondary() { return false; }
        
    void push(const Ty val, secondary_type&& sec = secondary_type() ) 
    {
        _count1 = 0;        
        _push( val);        
    }

    void push(const generic_type& gen, secondary_type&& sec = secondary_type() )
    {
        _count1 = 0;
        _push( (Ty)gen );        
    }
        
    void
    copy( Ty* dest, size_t sz, int end = -1, int beg = 0, secondary_type* sec = nullptr) const 
    {    
        static_assert( 
            !std::is_same<Ty,char>::value, 
            "->copy() accepts char**, not char*" 
            );
        if( !dest )
            throw invalid_argument( " ->copy(): *dest argument can not be null ");
        _yld_to_push();
        _guardTy _lock_( *_mtx );
        _check_adj( end, beg, _myImplObj );                     
        if( end == beg )
            *dest = beg ? _myImplObj.at(beg) : _myImplObj.front();
        else 
            _copy_to_ptr(_myImplObj,dest,sz,end,beg);         
    }
    
    /* slow(er), has to go thru generic_type to get strings */
    /* note: if sz <= genS.length() the string is truncated */
    void
    copy( char** dest, size_t destSz, size_t strSz, int end = -1, int beg = 0, secondary_type* sec = nullptr) const 
    {
        _myImplTy::const_iterator bIter, eIter;        
        if( !dest )
            throw invalid_argument( " ->copy(): *dest argument can not be null ");
        _yld_to_push();        
        _guardTy _lock_( *_mtx );                    
        _check_adj(end,beg, _myImplObj);                        
        bIter = _myImplObj.cbegin() + beg; 
        eIter = _myImplObj.cbegin() + std::min< size_t >(++end, _qCount);
        for( size_t i = 0; (i < destSz) && (bIter < eIter); ++bIter, ++i )
        {             
            std::string genS = generic_type( *bIter ).as_string();                
            strncpy_s( dest[i], strSz, genS.c_str(), std::min<size_t>(strSz-1,genS.length()));                                  
        }            
    }

    generic_type operator[]( int indx) const
    {
        int dummy = 0;
        _guardTy _lock_( *_mtx );
        if ( !indx ) /* optimize for indx == 0 */
            return generic_type( _myImplObj.front() ); 
        _check_adj( indx, dummy, _myImplObj );             
        return generic_type( _myImplObj.at(indx) );         
    }

    both_type both( int indx ) const                        
    {    
        _guardTy _lock_( *_mtx );
        return both_type( operator[](indx), secondary_type() );
    }

    generic_vector_type 
    vector(int end = -1, int beg = 0) const 
    {
        _myImplTy::const_iterator bIter, eIter;        
        generic_vector_type tmp;        
        _yld_to_push();        
        _guardTy _lock_( *_mtx );
        _check_adj(end, beg, _myImplObj);                
        bIter = _myImplObj.cbegin() + beg;
        eIter = _myImplObj.cbegin() + std::min< size_t >(++end, _qCount);        
        if( bIter < eIter )        
            std::transform(  /* have to use slower insert_iterator approach */
                    bIter,   /* generic_type doesn't allow default construction */                            
                    eIter, 
                    std::insert_iterator< generic_vector_type >(tmp,tmp.begin()), 
                    []( Ty x ){ return generic_type(x); } 
                    );            
        return tmp;    
    }

    secondary_vector_type
    secondary_vector( int end = -1, int beg = 0 ) const
    {                
        _check_adj(end, beg, _myImplObj);                                            
        return secondary_vector_type(  std::min< size_t >(++end - beg, _qCount) );
    }

};

/*    The container object w/ secondary deque */
template < typename Ty,            
            typename SecTy,
             typename GenTy,
              typename Allocator >
class Object< Ty, SecTy, GenTy, true, Allocator >
    : public Object< Ty, SecTy, GenTy, false, Allocator >{

    typedef Object< Ty, SecTy, GenTy, true, Allocator>  _myTy;
    typedef Object< Ty, SecTy, GenTy, false, Allocator> _myBase;
    typedef std::deque< SecTy, Allocator >              _myImplSecTy;
    
    _myImplSecTy _myImplSecObj;    
    
    void _push(const Ty _item, const secondary_type&& sec) 
    {    
        _push_has_priority = _mtx->try_lock();
        if( !_push_has_priority )
            _mtx->lock();
        _myBase::_myImplObj.push_front(_item); 
        _myBase::_myImplObj.pop_back();
        _myImplSecObj.push_front( std::move(sec) );
        _myImplSecObj.pop_back();
        if( _qCount < _qBound )
            ++_qCount;
        _mtx->unlock();
    } 

public:
    typedef Ty    value_type;

    Object(size_t sz )
        : 
        _myImplSecObj( std::min<size_t>(sz,MAX_BOUND_SIZE) ),
        _myBase( std::min<size_t>(sz,MAX_BOUND_SIZE) )
        {
        }

    Object(const _myTy & stream)
        : 
        _myImplSecObj( stream._myImplSecObj ),
        _myBase( stream )
        { 
        }

    Object(_myTy && stream)
        : 
        _myImplSecObj( std::move( stream._myImplSecObj) ),
        _myBase( std::move(stream) )
        {
        }

    size_t bound_size(size_t sz)
    {
        sz = std::min<size_t>(sz, MAX_BOUND_SIZE);
        _guardTy _lock_( *_mtx );        
        _myImplSecObj.resize(sz);
        if (sz < _qCount)
            _myImplSecObj.shrink_to_fit();    
        return _myBase::bound_size(sz);        
    }

    bool inline uses_secondary() { return true; }

    void push( const Ty val, secondary_type&& sec = secondary_type() ) 
    {        
        _count1 = 0;
        _push( val, std::move(sec) );    
    
    }

    void push( const generic_type& gen, secondary_type&& sec = secondary_type() )
    {
        _count1 = 0;
        _push( (Ty)gen, std::move(sec) );
    }
    
    void
    copy( Ty* dest, size_t sz, int end = -1, int beg = 0, secondary_type* sec = nullptr) const 
    {        
        if( !dest )
            throw invalid_argument( " ->copy(): *dest argument can not be null ");
        _guardTy _lock_( *_mtx );            
        _myBase::copy(dest, sz, end, beg);            
        if( !sec )
            return;
        _check_adj( end, beg, _myImplSecObj ); /*repeat the check to update index vals */        
        if( end == beg )    
            *sec = beg ? _myImplSecObj.at(beg) : _myImplSecObj.front();
        else    
            _copy_to_ptr( _myImplSecObj, sec, sz, end, beg);    
    }

    void
    copy( char** dest, size_t destSz, size_t strSz, int end = -1, int beg = 0, secondary_type* sec = nullptr) const 
    {            
        if( !dest  )
            throw invalid_argument( " ->copy(): *dest argument can not be null ");
        _guardTy _lock_( *_mtx );
        _myBase::copy(dest, destSz, strSz, end, beg);
        if( !sec )
            return;
        _check_adj( end, beg, _myImplSecObj ); /*repeat the check to update index vals*/
        if( end == beg )
            *sec = beg ? _myImplSecObj.at(beg) : _myImplSecObj.front();
        else
            _copy_to_ptr( _myImplSecObj, sec, destSz, end, beg);        
    }
    
    both_type both( int indx ) const                        
    {        
        int dummy = 0;        
        _guardTy _lock_( *_mtx );        
        generic_type gen = operator[](indx);
        if( !indx )
            return both_type( gen, _myImplSecObj.front() );
        _check_adj(indx, dummy, _myImplSecObj );            
        return both_type( gen, _myImplSecObj.at(indx) );
    }

    void secondary( secondary_type* dest, int indx ) const
    {
        int dummy = 0;
        _guardTy _lock_( *_mtx );
        _check_adj(indx, dummy, _myImplSecObj );
        if( !indx )
            *dest = _myImplSecObj.front();
        else
            *dest = _myImplSecObj.at( indx );        
    }

    secondary_vector_type
    secondary_vector( int end = -1, int beg = 0 ) const
    {
        _myImplSecTy::const_iterator bIter, eIter;
        _myImplSecTy::const_iterator::difference_type iterDiff;
        secondary_vector_type tmp;        
        _yld_to_push();        
        _guardTy _lock_( *_mtx );
        _check_adj(end, beg, _myImplSecObj);                
        bIter = _myImplSecObj.cbegin() + beg;
        eIter = _myImplSecObj.cbegin() + std::min< size_t >(++end, _qCount);    
        iterDiff = eIter - bIter;
        if( iterDiff > 0 )
        { /* do this manually; insert iterators too slow */
            tmp.resize(iterDiff); 
            std::copy( bIter, eIter, tmp.begin() );
        }
        return tmp;    
    }
};    
};

template< typename T, typename T2 >
std::ostream& 
operator<<(std::ostream&, const tosdb_data_stream::Interface<T,T2>&);

#endif