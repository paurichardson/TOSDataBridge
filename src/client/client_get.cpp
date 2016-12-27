/* 
Copyright (C) 2014 Jonathon Ogden   < jeog.dev@gmail.com >

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

#include "raw_data_block.hpp"
#include "client.hpp"
#include <ctime>
#include <fstream>
#include <iostream>
#include <iomanip>

size_type 
TOSDB_GetBlockLimit()
{   
    GLOBAL_RLOCK_GUARD;
    /* --- CRITICAL SECTION --- */
    return TOSDB_RawDataBlock::max_block_count();
    /* --- CRITICAL SECTION --- */
}

size_type 
TOSDB_SetBlockLimit(size_type sz)
{  
    GLOBAL_RLOCK_GUARD; 
    /* --- CRITICAL SECTION --- */
    return TOSDB_RawDataBlock::max_block_count(sz);
    /* --- CRITICAL SECTION --- */
}

size_type 
TOSDB_GetBlockCount()
{   
    GLOBAL_RLOCK_GUARD;  
    /* --- CRITICAL SECTION --- */
    return TOSDB_RawDataBlock::block_count();
    /* --- CRITICAL SECTION --- */
}

int 
TOSDB_GetBlockSize(LPCSTR id, size_type* pSize)
{
    try{   
        if(!CheckIDLength(id))
            return -1;

        GLOBAL_RLOCK_GUARD;  
        /* --- CRITICAL SECTION --- */
        *pSize = GetBlockOrThrow(id)->block->block_size();
        return 0;
        /* --- CRITICAL SECTION --- */
    }catch(...){ 
        return -2; 
    }
}

int 
TOSDB_SetBlockSize(LPCSTR id, size_type sz)
{
    try{
        if(!CheckIDLength(id))
            return -1;

        GLOBAL_RLOCK_GUARD;
        /* --- CRITICAL SECTION --- */
        GetBlockOrThrow(id)->block->block_size(sz);
        return 0;
        /* --- CRITICAL SECTION --- */
    }catch(...){ 
        return -2; 
    }
}

int 
TOSDB_GetItemCount(LPCSTR id, size_type* count)
{
    try{
        if(!CheckIDLength(id))
            return -1;

        GLOBAL_RLOCK_GUARD;
        /* --- CRITICAL SECTION --- */
        *count = GetBlockOrThrow(id)->block->item_count();
        return 0;
        /* --- CRITICAL SECTION --- */
    }
    catch(...){ 
        return -2; 
    }
}

int 
TOSDB_GetTopicCount(LPCSTR id, size_type* count)
{
    try{
        if(!CheckIDLength(id))
            return -1;

        GLOBAL_RLOCK_GUARD;
        /* --- CRITICAL SECTION --- */
        *count = GetBlockOrThrow(id)->block->topic_count();
        return 0;
        /* --- CRITICAL SECTION --- */
    }
    catch(...){ 
        return -2; 
    }
}



int 
TOSDB_GetTopicNames(LPCSTR id, LPSTR* dest, size_type array_len, size_type str_len)
{
    const TOSDBlock *db;
    topic_set_type topics;
    int i, err;

    if(!CheckIDLength(id))
        return -1;

    GLOBAL_RLOCK_GUARD;
    /* --- CRITICAL SECTION --- */
    db = GetBlockPtr(id);
    if (!db) 
        return -2;

    topics = db->block->topics();  
    if (array_len < topics.size()) 
        return -3;  

    i = err = 0;
    for(auto & t : topics){
        err = strcpy_s(dest[i++], str_len,TOS_Topics::map[t].c_str());
        if(err)
            return err;     
    }
    return err;
    /* --- CRITICAL SECTION --- */
}

int 
TOSDB_GetItemNames(LPCSTR id, LPSTR* dest, size_type array_len, size_type str_len)
{
    const TOSDBlock *db;
    str_set_type items;
    int i, err;

    if(!CheckIDLength(id))
        return -1;

    GLOBAL_RLOCK_GUARD;
    /* --- CRITICAL SECTION --- */
    db = GetBlockPtr(id);
    if (!db) 
        return -2;

    items = db->block->items();   
    if (array_len < items.size()) 
        return -3;   
 
    i = err = 0;
    for(auto & item : items){
        err = strcpy_s(dest[i++], str_len, item.c_str());
        if(err)
            return err;
    }
    return err;
    /* --- CRITICAL SECTION --- */
}


/*** --- BEGIN --- Oct 30 2016 ***/

int 
TOSDB_GetPreCachedItemCount(LPCSTR id, size_type* count)
{
    try{
        if(!CheckIDLength(id))
            return -1;

        GLOBAL_RLOCK_GUARD;
        /* --- CRITICAL SECTION --- */
        *count = GetBlockOrThrow(id)->item_precache.size();
        return 0;
        /* --- CRITICAL SECTION --- */
    }
    catch(...){ 
        return -2; 
    }
}

int 
TOSDB_GetPreCachedTopicCount(LPCSTR id, size_type* count)
{
    try{
        if(!CheckIDLength(id))
            return -1;

        GLOBAL_RLOCK_GUARD;
        /* --- CRITICAL SECTION --- */
        *count = GetBlockOrThrow(id)->topic_precache.size();
        return 0;
        /* --- CRITICAL SECTION --- */
    }
    catch(...){ 
        return -2; 
    }
}


int 
TOSDB_GetPreCachedTopicNames(LPCSTR id, LPSTR* dest, size_type array_len, size_type str_len)
{
    const TOSDBlock *db;    
    int i, err;

    if(!CheckIDLength(id))
        return -1;

    GLOBAL_RLOCK_GUARD;
    /* --- CRITICAL SECTION --- */
    db = GetBlockPtr(id);
    if (!db) 
        return -2;
        
    if (array_len < db->topic_precache.size()) 
        return -3;  

    i = err = 0;
    for(auto & t : db->topic_precache){
        err = strcpy_s(dest[i++], str_len,TOS_Topics::map[t].c_str());
        if(err)
            return err;     
    }
    return err;
    /* --- CRITICAL SECTION --- */
}

int 
TOSDB_GetPreCachedItemNames(LPCSTR id, LPSTR* dest, size_type array_len, size_type str_len)
{
    const TOSDBlock *db;    
    int i, err;

    if(!CheckIDLength(id))
        return -1;

    GLOBAL_RLOCK_GUARD;
    /* --- CRITICAL SECTION --- */
    db = GetBlockPtr(id);
    if (!db) 
        return -2;
        
    if (array_len < db->item_precache.size()) 
        return -3;   
 
    i = err = 0;
    for(auto & item : db->item_precache){
        err = strcpy_s(dest[i++], str_len, item.c_str());
        if(err)
            return err;
    }
    return err;
    /* --- CRITICAL SECTION --- */
}

/*** --- END --- Oct 30 2016 ***/


int 
TOSDB_GetTypeBits(LPCSTR topic_str, type_bits_type* type_bits)
{
    TOS_Topics::TOPICS t;

    if(!CheckStringLength(topic_str))
        return -1;

    t = GetTopicEnum(topic_str);
    if(t == TOS_Topics::TOPICS::NULL_TOPIC)
        return -2;

    try{
        *type_bits = TOSDB_GetTypeBits(t);
        return 0;
    }catch(...){ 
        return -3; 
    }
}

int 
TOSDB_GetTypeString(LPCSTR topic_str, LPSTR dest, size_type str_len)
{
    TOS_Topics::TOPICS t;
    std::string str;

    if(!CheckStringLength(topic_str))
        return -1;

    t = GetTopicEnum(topic_str);
    if(t == TOS_Topics::TOPICS::NULL_TOPIC)
        return -2;

    str = TOS_Topics::TypeString(t);
    return strcpy_s(dest, str_len, str.c_str());
}

int 
TOSDB_IsUsingDateTime(LPCSTR id, unsigned int* is_datetime)
{  
    try{
        if(!CheckIDLength(id))
            return -1;

        GLOBAL_RLOCK_GUARD;
        /* --- CRITICAL SECTION --- */
        *is_datetime = GetBlockOrThrow(id)->block->uses_dtstamp();
        return 0;
        /* --- CRITICAL SECTION --- */
    }catch(...){ 
        return -2; 
    }
}

topic_set_type 
TOSDB_GetTopicEnums(std::string id)
{
    const TOSDBlock *db;

    GLOBAL_RLOCK_GUARD;
    /* --- CRITICAL SECTION --- */
    db = GetBlockOrThrow(id);
    return db->block->topics();
    /* --- CRITICAL SECTION --- */
}

str_set_type 
TOSDB_GetTopicNames(std::string id)
{
    const TOSDBlock *db;  
    auto f = [&](TOS_Topics::TOPICS t){ return TOS_Topics::map[t]; };

    GLOBAL_RLOCK_GUARD;
    /* --- CRITICAL SECTION --- */
    db = GetBlockOrThrow(id);    
    return str_set_type(db->block->topics(), f);
  /* --- CRITICAL SECTION --- */
}

str_set_type 
TOSDB_GetItemNames(std::string id)
{
    const TOSDBlock* db;  

    GLOBAL_RLOCK_GUARD;
    /* --- CRITICAL SECTION --- */
    db = GetBlockOrThrow(id);
    return db->block->items();
    /* --- CRITICAL SECTION --- */
}

topic_set_type 
TOSDB_GetPreCachedTopicEnums(std::string id)
{
    const TOSDBlock *db;

    GLOBAL_RLOCK_GUARD;
    /* --- CRITICAL SECTION --- */
    db = GetBlockOrThrow(id);
    return db->topic_precache;
    /* --- CRITICAL SECTION --- */
}

str_set_type 
TOSDB_GetPreCachedTopicNames(std::string id)
{
    const TOSDBlock *db;  
    auto f = [=](TOS_Topics::TOPICS top){ return TOS_Topics::map[top]; };

    GLOBAL_RLOCK_GUARD;
    /* --- CRITICAL SECTION --- */
    db = GetBlockOrThrow(id);
    return str_set_type(db->topic_precache, f);
  /* --- CRITICAL SECTION --- */
}

str_set_type 
TOSDB_GetPreCachedItemNames(std::string id)
{
    const TOSDBlock *db;  

    GLOBAL_RLOCK_GUARD;
    /* --- CRITICAL SECTION --- */
    db = GetBlockOrThrow(id);
    return db->item_precache;
    /* --- CRITICAL SECTION --- */
}

type_bits_type 
TOSDB_GetTypeBits(TOS_Topics::TOPICS topic_t)
{
    if(topic_t == TOS_Topics::TOPICS::NULL_TOPIC)
        throw std::invalid_argument("NULL TOPIC");

    return TOS_Topics::TypeBits(topic_t);          
}

std::string 
TOSDB_GetTypeString(TOS_Topics::TOPICS topic_t)
{
    if(topic_t == TOS_Topics::TOPICS::NULL_TOPIC)
        throw std::invalid_argument("NULL TOPIC");

    return TOS_Topics::TypeString(topic_t);
}

size_type 
TOSDB_GetItemCount(std::string id)
{
    GLOBAL_RLOCK_GUARD;
    /* --- CRITICAL SECTION --- */
    return  GetBlockOrThrow(id)->block->item_count();
    /* --- CRITICAL SECTION --- */
}

size_type 
TOSDB_GetTopicCount(std::string id)
{
    GLOBAL_RLOCK_GUARD;
    /* --- CRITICAL SECTION --- */
    return GetBlockOrThrow(id)->block->topic_count();
    /* --- CRITICAL SECTION --- */
}


/*** --- BEGIN --- Oct 30 2016 ***/

size_type 
TOSDB_GetPreCachedItemCount(std::string id)
{
    GLOBAL_RLOCK_GUARD;
    /* --- CRITICAL SECTION --- */
    return  GetBlockOrThrow(id)->item_precache.size();
    /* --- CRITICAL SECTION --- */
}

size_type 
TOSDB_GetPreCachedTopicCount(std::string id)
{
    GLOBAL_RLOCK_GUARD;
    /* --- CRITICAL SECTION --- */
    return GetBlockOrThrow(id)->topic_precache.size();
    /* --- CRITICAL SECTION --- */
}

/*** --- END --- Oct 30 2016 ***/


void 
TOSDB_SetBlockSize(std::string id, size_type sz)
{
    GLOBAL_RLOCK_GUARD;
    /* --- CRITICAL SECTION --- */
    GetBlockOrThrow(id)->block->block_size(sz);  
    /* --- CRITICAL SECTION --- */
}

size_type 
TOSDB_GetBlockSize(std::string id)
{
    GLOBAL_RLOCK_GUARD;
    /* --- CRITICAL SECTION --- */
    return GetBlockOrThrow(id)->block->block_size();  
    /* --- CRITICAL SECTION --- */
}

bool 
TOSDB_IsUsingDateTime(std::string id)
{  
    GLOBAL_RLOCK_GUARD;
    /* --- CRITICAL SECTION --- */
    return GetBlockOrThrow(id)->block->uses_dtstamp();
    /* --- CRITICAL SECTION --- */
}

int 
TOSDB_GetStreamOccupancy(LPCSTR id, LPCSTR item, LPCSTR topic_str, size_type* sz)
{
    const TOSDBlock *db;  
    TOSDB_RawDataBlock::stream_const_ptr_type dat;
    TOS_Topics::TOPICS t;

    if( !CheckIDLength(id) 
        || !CheckStringLength(item) 
        || !CheckStringLength(topic_str) ){
        return -1;  
    }

    t = GetTopicEnum(topic_str);
    try{
        GLOBAL_RLOCK_GUARD;
        /* --- CRITICAL SECTION --- */
        db = GetBlockOrThrow(id);
        dat = db->block->raw_stream_ptr(item, t);
        *sz = (size_type)(dat->size());
        return 0;
        /* --- CRITICAL SECTION --- */
    }catch(const std::exception& e){
        TOSDB_LogH("TOSDB_GetStreamOccupancy", e.what());
        return -2;
    }catch(...){ 
        return -2; 
    }  
}

size_type 
TOSDB_GetStreamOccupancy(std::string id, 
                         std::string item, 
                         TOS_Topics::TOPICS topic_t)
{
    const TOSDBlock *db;  
    TOSDB_RawDataBlock::stream_const_ptr_type dat;

    GLOBAL_RLOCK_GUARD;
    /* --- CRITICAL SECTION --- */
    db = GetBlockOrThrow(id);
    dat = db->block->raw_stream_ptr(item, topic_t);  
    
    try{
        return (size_type)(dat->size());
    }catch(const DataStreamError& e){
        throw TOSDB_DataStreamError(e, "TOSDB_GetStreamOccupancy");
    }
    /* --- CRITICAL SECTION --- */
}

int 
TOSDB_GetMarkerPosition(LPCSTR id, LPCSTR item, LPCSTR topic_str, long long* pos)
{
    const TOSDBlock *db;
    TOSDB_RawDataBlock::stream_const_ptr_type dat;
    TOS_Topics::TOPICS t;

    if( !CheckIDLength(id) 
        || !CheckStringLength(item)
        || !CheckStringLength(topic_str) ){
        return -1;  
    }

    t = GetTopicEnum(topic_str);
    try{
        GLOBAL_RLOCK_GUARD;
        /* --- CRITICAL SECTION --- */
        db = GetBlockOrThrow(id);
        dat = db->block->raw_stream_ptr(item, t);
        *pos = (dat->marker_position());
        return 0;
        /* --- CRITICAL SECTION --- */
    }catch(const std::exception& e){
        TOSDB_LogH("TOSDB_GetMarkerPosition", e.what());
        return -2;
    }catch(...){ 
        return -2; 
    }  
}

long long 
TOSDB_GetMarkerPosition(std::string id, 
                        std::string item, 
                        TOS_Topics::TOPICS topic_t)
{
    const TOSDBlock *db;  
    TOSDB_RawDataBlock::stream_const_ptr_type dat;

    GLOBAL_RLOCK_GUARD;
    /* --- CRITICAL SECTION --- */
    db = GetBlockOrThrow(id);
    dat = db->block->raw_stream_ptr(item, topic_t);      
    try{
        return dat->marker_position();
    }catch(const DataStreamError& e){
        throw TOSDB_DataStreamError(e, "TOSDB_GetMarkerPosition");
    }
    /* --- CRITICAL SECTION --- */
}

int 
TOSDB_IsMarkerDirty(LPCSTR id,
                    LPCSTR item, 
                    LPCSTR topic_str, 
                    unsigned int* is_dirty)
{
    const TOSDBlock *db;
    TOSDB_RawDataBlock::stream_const_ptr_type dat;
    TOS_Topics::TOPICS t;

    if( !CheckIDLength(id) 
        || !CheckStringLength(item)
        || !CheckStringLength(topic_str) ){ 
        return -1;  
    }

    t = GetTopicEnum(topic_str);
    try{
        GLOBAL_RLOCK_GUARD;
        /* --- CRITICAL SECTION --- */
        db = GetBlockOrThrow(id);
        dat = db->block->raw_stream_ptr(item, t);
        *is_dirty = (unsigned int)(dat->is_marker_dirty());
        return 0;
        /* --- CRITICAL SECTION --- */
    }catch(const std::exception& e){
        TOSDB_LogH("TOSDB_IsMarkerDirty", e.what());
        return -2;
    }catch(...){ 
        return -2; 
    }  
}

bool 
TOSDB_IsMarkerDirty(std::string id, 
                    std::string item, 
                    TOS_Topics::TOPICS topic_t)
{
    const TOSDBlock *db;  
    TOSDB_RawDataBlock::stream_const_ptr_type dat;

    GLOBAL_RLOCK_GUARD;
    /* --- CRITICAL SECTION --- */
    db = GetBlockOrThrow(id);
    dat = db->block->raw_stream_ptr(item, topic_t);  
    try{
        return dat->is_marker_dirty();
    }catch(const DataStreamError& e){
        throw TOSDB_DataStreamError(e, "TOSDB_IsMarkerDirty");
    }        
    /* --- CRITICAL SECTION --- */
}

template<> 
generic_type 
TOSDB_Get<generic_type, false>(std::string id, 
                               std::string item, 
                               TOS_Topics::TOPICS topic_t, 
                               long indx)
{  
    const TOSDBlock *db;  
    TOSDB_RawDataBlock::stream_const_ptr_type dat;

    GLOBAL_RLOCK_GUARD;
    /* --- CRITICAL SECTION --- */
    db = GetBlockOrThrow(id);
    dat = db->block->raw_stream_ptr(item, topic_t);  
    try{
        return dat->operator[](indx);
    }catch(const DataStreamError& e){
        throw TOSDB_DataStreamError(e, "TOSDB_Get<generic_type,false>");
    }
    /* --- CRITICAL SECTION --- */
}

template<> 
generic_dts_type 
TOSDB_Get<generic_type, true>(std::string id, 
                              std::string item, 
                              TOS_Topics::TOPICS topic_t, 
                              long indx)
{
    const TOSDBlock *db;
    TOSDB_RawDataBlock::stream_const_ptr_type dat;

    GLOBAL_RLOCK_GUARD;
    /* --- CRITICAL SECTION --- */
    db = GetBlockOrThrow(id);
    dat = db->block->raw_stream_ptr(item, topic_t);
    try{
        return dat->both(indx);
    }catch(const DataStreamError& e){
        throw TOSDB_DataStreamError(e, "TOSDB_Get<generic_type,true>");
    }    
    /* --- CRITICAL SECTION --- */
}

/* use functors of partially specialized object to overload by explicit call */
template<typename T, bool b> 
struct GRetType;

template<typename T>
struct GRetType<T, true>{
    std::pair<T, DateTimeStamp> 
    operator()(T val, DateTimeStamp dts)
    {      
        return std::pair<T, DateTimeStamp>(val, std::move(dts));
    }
};

template<typename T>
struct GRetType<T, false>{
    T 
    operator()(T val, DateTimeStamp dts) 
    { 
        return val; 
    }
};

template<typename T, bool b> 
auto 
TOSDB_Get(std::string id, 
          std::string item, 
          TOS_Topics::TOPICS topic_t, 
          long indx) -> typename std::conditional<b, std::pair<T, DateTimeStamp>, T>::type
{
    T tmp;
    DateTimeStamp datetime;

    if(TOSDB_Get_(id, item, topic_t, indx, &tmp, &datetime))    
        throw TOSDB_DataStreamError("TOSDB_Get_");

    return GRetType<T,b>()(tmp, std::move(datetime));  
}

template<typename T> 
int 
TOSDB_Get_(std::string id, 
           std::string item, 
           TOS_Topics::TOPICS topic_t, 
           long indx, 
           T* dest, 
           pDateTimeStamp datetime)
{   
    const TOSDBlock *db;
    TOSDB_RawDataBlock::stream_const_ptr_type dat; 
   
    try{
        GLOBAL_RLOCK_GUARD;
        /* --- CRITICAL SECTION --- */
        db = GetBlockOrThrow(id);
        dat = db->block->raw_stream_ptr(item, topic_t);  
        dat->copy(dest, 1,indx, indx, datetime);
        return 0;
        /* --- CRITICAL SECTION --- */
    }catch(const std::exception& e){
        TOSDB_LogH("TOSDB_Get<T>", e.what());
        return -1;
    }catch(...){
        return -1;
    }
}

template<typename T> 
int 
TOSDB_Get_(LPCSTR id, 
           LPCSTR item, 
           LPCSTR topic_str, 
           long indx, 
           T* dest, 
           pDateTimeStamp datetime)
{ 
    if( !CheckIDLength(id) 
        || !CheckStringLength(item)
        || !CheckStringLength(topic_str) ){
        return -1;  
    }
  
    return TOSDB_Get_(id, item, GetTopicEnum(topic_str), indx, dest, datetime);
}

int 
TOSDB_GetDouble(LPCSTR id, 
                LPCSTR item, 
                LPCSTR topic_str, 
                long indx, 
                ext_price_type* dest, 
                pDateTimeStamp datetime)
{  
    return TOSDB_Get_(id, item, topic_str , indx, dest, datetime);
}

int 
TOSDB_GetFloat(LPCSTR id, 
               LPCSTR item, 
               LPCSTR topic_str, 
               long indx, 
               def_price_type* dest, 
               pDateTimeStamp datetime)
{
    return TOSDB_Get_(id, item, topic_str , indx, dest, datetime);
}

int 
TOSDB_GetLongLong(LPCSTR id, 
                  LPCSTR item, 
                  LPCSTR topic_str, 
                  long indx, 
                  ext_size_type* dest, 
                  pDateTimeStamp datetime)
{
    return TOSDB_Get_(id, item, topic_str , indx, dest, datetime);
}

int 
TOSDB_GetLong(LPCSTR id, 
              LPCSTR item, 
              LPCSTR topic_str, 
              long indx, 
              def_size_type* dest, 
              pDateTimeStamp datetime)
{
    return TOSDB_Get_(id, item, topic_str , indx, dest, datetime);
}

int 
TOSDB_GetString(LPCSTR id, 
                LPCSTR item, 
                LPCSTR topic_str, 
                long indx, 
                LPSTR dest, 
                size_type str_len, 
                pDateTimeStamp datetime)
{  
    const TOSDBlock *db;
    TOSDB_RawDataBlock::stream_const_ptr_type dat;
    TOS_Topics::TOPICS t;

    if( !CheckIDLength(id) 
        || !CheckStringLength(item)
        || !CheckStringLength(topic_str) ){ 
        return -1;  
    }

    t = GetTopicEnum(topic_str);
    try{
        GLOBAL_RLOCK_GUARD;
        /* --- CRITICAL SECTION --- */
        db = GetBlockOrThrow(id);
        dat = db->block->raw_stream_ptr(item, t);
        dat->copy(&dest, 1, str_len, indx,indx,datetime);
        return 0;
        /* --- CRITICAL SECTION --- */
    }catch(const std::exception& e){
        TOSDB_LogH("TOSDB_GetString", e.what());
        return -2;
    }catch(...){ 
        return -2; 
    }  
}

template<> 
generic_vector_type 
TOSDB_GetStreamSnapshot<generic_type, false>(std::string id, 
                                             std::string item, 
                                             TOS_Topics::TOPICS topic_t, 
                                             long end, 
                                             long beg)
{
    const TOSDBlock *db;
    TOSDB_RawDataBlock::stream_const_ptr_type dat;

    GLOBAL_RLOCK_GUARD;
    /* --- CRITICAL SECTION --- */
    db = GetBlockOrThrow(id);  
    dat = db->block->raw_stream_ptr(item, topic_t);  
    try{
        return dat->vector(end, beg); 
    }catch(const DataStreamError& e){
        throw TOSDB_DataStreamError(e,"TOSDB_GetStreamSnapshot<generic_type,false>");
    }    
    /* --- CRITICAL SECTION --- */
}

template<> 
std::pair<std::vector<generic_type>,std::vector<DateTimeStamp>>     
TOSDB_GetStreamSnapshot<generic_type, true>(std::string id, 
                                            std::string item, 
                                            TOS_Topics::TOPICS topic_t, 
                                            long end, 
                                            long beg)
{
    const TOSDBlock *db;
    TOSDB_RawDataBlock::stream_const_ptr_type dat;

    GLOBAL_RLOCK_GUARD;
    /* --- CRITICAL SECTION --- */
    db = GetBlockOrThrow(id);  
    dat = db->block->raw_stream_ptr(item, topic_t);
    try{
        return std::pair<std::vector<generic_type>,
                         std::vector<DateTimeStamp>>(
                                dat->vector(end, beg), 
                                dat->secondary_vector(end,beg)
                         );
    }catch(const DataStreamError& e){
        throw TOSDB_DataStreamError(e, "TOSDB_GetStreamSnapshot<generic_type,true>");
    }    
    /* --- CRITICAL SECTION --- */
}

/* 
   use functors of partially specialized object to overload by explicit call
   note: data_stream takes raw pointers so we need to pass the addr of the 
   dereferenced first element of the vector 

   ARE WE RELYING ON VECTOR IMPLEMENTATION ??
*/
template<typename T, bool b> 
struct GSSRetType;

template<typename T>
struct GSSRetType<T, true>{
    std::pair<std::vector<T>, std::vector<DateTimeStamp>> 
    operator()(TOSDB_RawDataBlock::stream_const_ptr_type dat, 
               long end, 
               long beg, 
               size_t diff)
    {        
        std::vector<T> v(diff); /* adjust for [) -> [ ] */
        std::vector<DateTimeStamp> dtsv(diff); /* adjust for [) -> [ ] */  
        if(diff > 0)
            dat->copy(&(*(v.begin())), diff, end, beg, &(*(dtsv.begin())));  
   
        return std::pair<std::vector<T>,std::vector<DateTimeStamp>>(v, dtsv);
    }
};

template<typename T>
struct GSSRetType<T, false>{
    std::vector<T> 
    operator()(TOSDB_RawDataBlock::stream_const_ptr_type dat, 
               long end, 
               long beg, 
               size_t diff)
    {  
        std::vector<T> v(diff); /* adjust for [) -> [ ] */
        if(diff > 0)
            dat->copy(&(*(v.begin())), diff, end, beg, nullptr); 
     
        return v;
    }
};

template<typename T, bool b> 
auto 
TOSDB_GetStreamSnapshot(std::string id, 
                        std::string item, 
                        TOS_Topics::TOPICS topic_t, 
                        long end, 
                        long beg) 
    -> typename std::conditional<b, 
                                 std::pair<std::vector<T>, std::vector<DateTimeStamp>>, 
                                 std::vector<T>>::type
{ 
    size_type sz;
    long diff;  
    long long mindiff;
    const TOSDBlock *db;
    TOSDB_RawDataBlock::stream_const_ptr_type dat;

    GLOBAL_RLOCK_GUARD;
    /* --- CRITICAL SECTION --- */
    db = GetBlockOrThrow(id);  
    dat = db->block->raw_stream_ptr(item, topic_t); /* get stream size */
    sz = (size_type)(dat->bound_size());

    if(end < 0) 
        end += sz;
    if(beg < 0) 
        beg += sz;
    diff = end - beg;
    if(diff < 0)
        throw std::invalid_argument("end < beg");
    
    mindiff = (long long)std::min<size_t>((size_t)end + 1, dat->size()) - beg;
    if(mindiff < 0)
        mindiff = 0;
    else if(mindiff > TOSDB_MAX_BLOCK_SZ) /* protect the size_t cast(below) */
        throw std::invalid_argument("mindiff > TOSDB_MAX_BLOCK_SZ");

    try{ 
        return GSSRetType<T,b>()(dat, end, beg, (size_t)mindiff);
    }catch(const DataStreamError& e){
        throw TOSDB_DataStreamError(e, "TOSDB_GetStreamSnapshot<T,b>");
    }  
    /* --- CRITICAL SECTION --- */
}

template<typename T> 
int 
TOSDB_GetStreamSnapshot_(LPCSTR id,
                         LPCSTR item, 
                         TOS_Topics::TOPICS topic_t, 
                         T* dest, 
                         size_type array_len, 
                         pDateTimeStamp datetime, 
                         long end, 
                         long beg)
{
    const TOSDBlock *db;
    TOSDB_RawDataBlock::stream_const_ptr_type dat;

    if(!CheckIDLength(id) || !CheckStringLength(item))
        return -1;

    try{
        GLOBAL_RLOCK_GUARD;
        /* --- CRITICAL SECTION --- */
        db = GetBlockOrThrow(id);
        dat = db->block->raw_stream_ptr(item, topic_t);
        dat->copy(dest,array_len,end,beg,datetime);
        return 0;
        /* --- CRITICAL SECTION --- */
    }catch(const std::exception& e){
        TOSDB_LogH("GetStreamSnapshot<T>", e.what());
        return -2;
    }catch(...){ 
        return -2; 
    }
}

template<typename T> 
int 
TOSDB_GetStreamSnapshot_(LPCSTR id,
                         LPCSTR item, 
                         LPCSTR topic_str, 
                         T* dest, 
                         size_type array_len, 
                         pDateTimeStamp datetime, 
                         long end, 
                         long beg)
{  
    if(!CheckStringLength(topic_str)) /* let this go thru std::string ? */
        return -1;   
   
    return TOSDB_GetStreamSnapshot_(id, item, GetTopicEnum(topic_str), dest, 
                                    array_len, datetime, end, beg);
}

int 
TOSDB_GetStreamSnapshotDoubles(LPCSTR id,
                               LPCSTR item, 
                               LPCSTR topic_str, 
                               ext_price_type* dest, 
                               size_type array_len, 
                               pDateTimeStamp datetime, 
                               long end, 
                               long beg)
{
    return TOSDB_GetStreamSnapshot_(id, item, topic_str, dest, array_len, 
                                    datetime, end, beg);
}

int 
TOSDB_GetStreamSnapshotFloats(LPCSTR id, 
                              LPCSTR item, 
                              LPCSTR topic_str, 
                              def_price_type* dest, 
                              size_type array_len, 
                              pDateTimeStamp datetime, 
                              long end, 
                              long beg)
{
    return TOSDB_GetStreamSnapshot_(id, item, topic_str, dest, array_len, 
                                    datetime, end, beg);
}

int 
TOSDB_GetStreamSnapshotLongLongs(LPCSTR id, 
                                 LPCSTR item, 
                                 LPCSTR topic_str, 
                                 ext_size_type* dest, 
                                 size_type array_len, 
                                 pDateTimeStamp datetime, 
                                 long end, 
                                 long beg)
{
    return TOSDB_GetStreamSnapshot_(id, item, topic_str, dest, array_len, 
                                    datetime, end, beg);  
}

int 
TOSDB_GetStreamSnapshotLongs(LPCSTR id, 
                             LPCSTR item, 
                             LPCSTR topic_str, 
                             def_size_type* dest, 
                             size_type array_len, 
                             pDateTimeStamp datetime, 
                             long end, 
                             long beg)
{ 
    return TOSDB_GetStreamSnapshot_(id, item, topic_str, dest, array_len, 
                                    datetime, end, beg);  
}

int 
TOSDB_GetStreamSnapshotStrings(LPCSTR id, 
                               LPCSTR item, 
                               LPCSTR topic_str, 
                               LPSTR* dest, 
                               size_type array_len, 
                               size_type str_len, 
                               pDateTimeStamp datetime, 
                               long end, 
                               long beg)
{
    const TOSDBlock *db;
    TOSDB_RawDataBlock::stream_const_ptr_type dat;
    TOS_Topics::TOPICS topic_t;

    if( !CheckIDLength(id) 
        || !CheckStringLength(item)
        || !CheckStringLength(topic_str) ){
        return -1;
    }

    topic_t = GetTopicEnum(topic_str);    
    try{
        GLOBAL_RLOCK_GUARD;
        /* --- CRITICAL SECTION --- */
        db = GetBlockOrThrow(id);
        dat = db->block->raw_stream_ptr(item, topic_t);
        dat->copy(dest,array_len,str_len,end,beg,datetime);
        return 0;
        /* --- CRITICAL SECTION --- */
    }catch(const std::exception& e){
        TOSDB_LogH("TOSDB_GetStreamSnapshotStrings", e.what());
        return -2;
    }catch(...){ 
        return -2; 
    }
}

template<typename T> 
int 
TOSDB_GetStreamSnapshotFromMarker_(LPCSTR id,
                                   LPCSTR item, 
                                   TOS_Topics::TOPICS topic_t, 
                                   T* dest, 
                                   size_type array_len, 
                                   pDateTimeStamp datetime,                     
                                   long beg,
                                   long *get_size)
{
    const TOSDBlock *db;
    TOSDB_RawDataBlock::stream_const_ptr_type dat;

    if(!CheckIDLength(id) || !CheckStringLength(item))
        return -1;

    try{
        GLOBAL_RLOCK_GUARD;
        /* --- CRITICAL SECTION --- */
        db = GetBlockOrThrow(id);
        dat = db->block->raw_stream_ptr(item, topic_t);
                   /* O.K. as long as data_stream::MAX_BOUND_SIZE == INT_MAX */
        *get_size = (long)(dat->copy_from_marker(dest,array_len,beg,datetime));
        return 0;
        /* --- CRITICAL SECTION --- */
    }catch(const std::exception& e){
        TOSDB_LogH("GetStreamSnapshotFromMarker<T>", e.what());
        return -2;
    }catch(...){ 
        return -2; 
    }
}

template<typename T> 
int 
TOSDB_GetStreamSnapshotFromMarker_(LPCSTR id,
                                   LPCSTR item, 
                                   LPCSTR topic_str, 
                                   T* dest, 
                                   size_type array_len, 
                                   pDateTimeStamp datetime,               
                                   long beg,
                                   long *get_size)
{  
    if(!CheckStringLength(topic_str)) /* let this go thru std::string ? */
        return 0;   
   
    return TOSDB_GetStreamSnapshotFromMarker_(id,item,GetTopicEnum(topic_str),dest,
                                              array_len, datetime, beg, get_size);
}

int 
TOSDB_GetStreamSnapshotDoublesFromMarker(LPCSTR id,
                                         LPCSTR item, 
                                         LPCSTR topic_str, 
                                         ext_price_type* dest, 
                                         size_type array_len, 
                                         pDateTimeStamp datetime,                         
                                         long beg,
                                         long *get_size)
{
    return TOSDB_GetStreamSnapshotFromMarker_(id, item, topic_str, dest, array_len, 
                                              datetime, beg, get_size);
}

int 
TOSDB_GetStreamSnapshotFloatsFromMarker(LPCSTR id, 
                                        LPCSTR item, 
                                        LPCSTR topic_str, 
                                        def_price_type* dest, 
                                        size_type array_len, 
                                        pDateTimeStamp datetime,                        
                                        long beg,
                                        long *get_size)
{
    return TOSDB_GetStreamSnapshotFromMarker_(id, item, topic_str, dest, array_len, 
                                              datetime, beg, get_size);
}

int 
TOSDB_GetStreamSnapshotLongLongsFromMarker(LPCSTR id, 
                                           LPCSTR item, 
                                           LPCSTR topic_str, 
                                           ext_size_type* dest, 
                                           size_type array_len, 
                                           pDateTimeStamp datetime,                         
                                           long beg,
                                           long *get_size)
{
    return TOSDB_GetStreamSnapshotFromMarker_(id, item, topic_str, dest, array_len, 
                                              datetime, beg, get_size);  
}

int 
TOSDB_GetStreamSnapshotLongsFromMarker(LPCSTR id, 
                                       LPCSTR item, 
                                       LPCSTR topic_str, 
                                       def_size_type* dest, 
                                       size_type array_len, 
                                       pDateTimeStamp datetime,                        
                                       long beg,
                                       long *get_size)
{
    return TOSDB_GetStreamSnapshotFromMarker_(id, item, topic_str, dest, array_len, 
                                              datetime, beg, get_size);  
}

int 
TOSDB_GetStreamSnapshotStringsFromMarker(LPCSTR id, 
                                         LPCSTR item, 
                                         LPCSTR topic_str, 
                                         LPSTR* dest, 
                                         size_type array_len, 
                                         size_type str_len, 
                                         pDateTimeStamp datetime,                         
                                         long beg,
                                         long *get_size)
{
    const TOSDBlock *db;
    TOSDB_RawDataBlock::stream_const_ptr_type dat;
    TOS_Topics::TOPICS topic_t;

    if( !CheckIDLength(id) 
        || !CheckStringLength(item)
        || !CheckStringLength(topic_str) ){
        return -1;
    }

    topic_t = GetTopicEnum(topic_str);   
    try{
        GLOBAL_RLOCK_GUARD;
        /* --- CRITICAL SECTION --- */
        db = GetBlockOrThrow(id);
        dat = db->block->raw_stream_ptr(item, topic_t);
                    /* O.K. as long as data_stream::MAX_BOUND_SIZE == INT_MAX */
        *get_size = (long)(dat->copy_from_marker(dest, array_len, str_len, beg, datetime));   
        return 0;
        /* --- CRITICAL SECTION --- */
    }catch(const std::exception& e){
        TOSDB_LogH("TOSDB_GetStreamSnapshotStringsFromMarker", e.what());
        return -2;
    }catch(...){ 
        return -2; 
    }
}

template<> 
generic_map_type 
TOSDB_GetItemFrame<false>(std::string id, TOS_Topics::TOPICS topic_t)
{
    const TOSDBlock *db; 

    GLOBAL_RLOCK_GUARD;
    /* --- CRITICAL SECTION --- */
    db = GetBlockOrThrow(id);
    return db->block->map_of_frame_items(topic_t);
    /* --- CRITICAL SECTION --- */
}

template<> 
generic_dts_map_type 
TOSDB_GetItemFrame<true>(std::string id, TOS_Topics::TOPICS topic_t)
{
    const TOSDBlock *db; 

    GLOBAL_RLOCK_GUARD;
    /* --- CRITICAL SECTION --- */
    db = GetBlockOrThrow(id);
    return db->block->pair_map_of_frame_items(topic_t);
    /* --- CRITICAL SECTION --- */
}

template<typename T> 
int 
TOSDB_GetItemFrame_(LPCSTR id, 
                    TOS_Topics::TOPICS topic_t, 
                    T* dest, 
                    size_type array_len, 
                    LPSTR* dest2, 
                    size_type str_len2, 
                    pDateTimeStamp datetime)
{
    const TOSDBlock *db;
    int err = 0;

    if(!CheckIDLength(id))
        return -1;

    try{
        GLOBAL_RLOCK_GUARD;
        /* --- CRITICAL SECTION --- */
        db = GetBlockOrThrow(id);

        if(datetime){  
            auto dtsm = db->block->pair_map_of_frame_items(topic_t);
            auto b_iter = dtsm.cbegin();
            auto e_iter = dtsm.cend();

            for( size_type i = 0; 
                 (i < array_len) && (b_iter != e_iter); 
                 ++b_iter, ++i )
            {
                dest[i] = (T)b_iter->second.first;
                datetime[i] = b_iter->second.second;

                if(dest2){
                    err = strcpy_s(dest2[i], str_len2, (b_iter->first).c_str());
                    if(err) 
                        return err; 
                }
            }      
        }else{        
            auto m = db->block->map_of_frame_items(topic_t);
            auto b_iter = m.cbegin();
            auto e_iter = m.cend();   
           
            for( size_type i = 0; 
                 (i < array_len) && (b_iter != e_iter); 
                 ++b_iter, ++i )  
            {      
                dest[i] = (T)b_iter->second; 
 
                if(dest2){
                    err = strcpy_s(dest2[i], str_len2, (b_iter->first).c_str());
                    if(err) 
                        return err;

                }
            }            
        }    
        /* --- CRITICAL SECTION --- */
    }catch(const std::exception& e){
        TOSDB_LogH("GetItemFrame<T>", e.what());
        return -2;  
    }catch(...){ 
        return -2; 
    }

  return 0;
}

template<typename T> 
int 
TOSDB_GetItemFrame_(LPCSTR id, 
                    LPCSTR topic_str, 
                    T* dest, 
                    size_type array_len, 
                    LPSTR* dest2, 
                    size_type str_len2, 
                    pDateTimeStamp datetime)
{  
    if(!CheckStringLength(topic_str))
        return -1;   
   
    return TOSDB_GetItemFrame_(id, GetTopicEnum(topic_str), dest, array_len, 
                               dest2, str_len2, datetime);  
}

int 
TOSDB_GetItemFrameDoubles(LPCSTR id, 
                          LPCSTR topic_str, 
                          ext_price_type* dest, 
                          size_type array_len, 
                          LPSTR* label_dest, 
                          size_type label_str_len, 
                          pDateTimeStamp datetime)
{
    return TOSDB_GetItemFrame_(id, topic_str, dest, array_len, label_dest, 
                               label_str_len, datetime);    
}

int 
TOSDB_GetItemFrameFloats(LPCSTR id, 
                         LPCSTR topic_str, 
                         def_price_type* dest, 
                         size_type array_len, 
                         LPSTR* label_dest, 
                         size_type label_str_len, 
                         pDateTimeStamp datetime)
{
    return TOSDB_GetItemFrame_(id, topic_str, dest, array_len, label_dest, 
                               label_str_len, datetime);      
}

int 
TOSDB_GetItemFrameLongLongs(LPCSTR id, 
                            LPCSTR topic_str, 
                            ext_size_type* dest, 
                            size_type array_len, 
                            LPSTR* label_dest, 
                            size_type label_str_len, 
                            pDateTimeStamp datetime)
{
    return TOSDB_GetItemFrame_(id, topic_str, dest, array_len, label_dest, 
                               label_str_len, datetime);      
}

int 
TOSDB_GetItemFrameLongs(LPCSTR id, 
                        LPCSTR topic_str, 
                        def_size_type* dest, 
                        size_type array_len,
                        LPSTR* label_dest, 
                        size_type label_str_len, 
                        pDateTimeStamp datetime)
{ 
    return TOSDB_GetItemFrame_(id, topic_str, dest, array_len, label_dest, 
                               label_str_len, datetime);      
}

int 
TOSDB_GetItemFrameStrings(LPCSTR id, 
                          LPCSTR topic_str, 
                          LPSTR* dest,  
                          size_type array_len, 
                          size_type str_len, 
                          LPSTR* label_dest, 
                          size_type label_str_len, 
                          pDateTimeStamp datetime)
{  
    const TOSDBlock *db;
    TOS_Topics::TOPICS topic_t;

    int err = 0;

    if(!CheckIDLength(id) || !CheckStringLength(topic_str))
      return -1;

    topic_t = GetTopicEnum(topic_str);   
    try{
        GLOBAL_RLOCK_GUARD;
        /* --- CRITICAL SECTION --- */
        db = GetBlockOrThrow(id);

        if(datetime){      
            auto dtsm = db->block->pair_map_of_frame_items(topic_t);
            auto b_iter = dtsm.cbegin();
            auto e_iter = dtsm.cend();        
     
            for( size_type i = 0; 
                 (i < array_len) && (b_iter != e_iter); 
                  ++b_iter, ++i)
            {
                datetime[i] = b_iter->second.second;

                err = strcpy_s(dest[i], str_len, (b_iter->second.first).as_string().c_str());
                if(err) 
                    return err;   
 
                if(label_dest){
                    err = strcpy_s(label_dest[i], label_str_len, (b_iter->first).c_str());
                    if(err)              
                        return err;
                }
            }    
        }else{          
            auto m = db->block->map_of_frame_items(topic_t);
            auto b_iter = m.cbegin();
            auto e_iter = m.cend();   
         
            for( size_type i = 0; 
                 (i < array_len) && (b_iter != e_iter); 
                 ++b_iter, ++i )
            {
                err = strcpy_s(dest[i], str_len, (b_iter->second).as_string().c_str());
                if(err)
                    return err;

                if(label_dest){
                    err = strcpy_s(label_dest[i], label_str_len, (b_iter->first).c_str());             
                    if(err)
                        return err; 
                }
            }
        }    
       /* --- CRITICAL SECTION --- */
    }catch(const std::exception& e){
        TOSDB_LogH("TOSDB_GetItemFrameStrings", e.what());
        return -2;
    }catch(...){ 
        return -2; 
    }

  return 0;    
} 

template<> 
generic_map_type 
TOSDB_GetTopicFrame<false>(std::string id, std::string item)
{
    const TOSDBlock *db;

    GLOBAL_RLOCK_GUARD;
    /* --- CRITICAL SECTION --- */
    db = GetBlockOrThrow(id);
    return db->block->map_of_frame_topics(item);
    /* --- CRITICAL SECTION --- */
}

template<> 
generic_dts_map_type 
TOSDB_GetTopicFrame<true>(std::string id,std::string item)
{
    const TOSDBlock *db;

    GLOBAL_RLOCK_GUARD;
    /* --- CRITICAL SECTION --- */
    db = GetBlockOrThrow(id);
    return db->block->pair_map_of_frame_topics(item);
    /* --- CRITICAL SECTION --- */
}

int 
TOSDB_GetTopicFrameStrings(LPCSTR id, 
                           LPCSTR item, 
                           LPSTR* dest, 
                           size_type array_len, 
                           size_type str_len, 
                           LPSTR* label_dest, 
                           size_type label_str_len, 
                           pDateTimeStamp datetime)
{  
    const TOSDBlock *db;
    int err = 0;

    if(!CheckIDLength(id) || !CheckStringLength(item))
        return -1;  

    try{
        GLOBAL_RLOCK_GUARD;
        /* --- CRITICAL SECTION --- */
        db = GetBlockOrThrow(id);

        if(datetime){       
            auto dtsm = db->block->pair_map_of_frame_topics(item);
            auto b_iter = dtsm.cbegin();
            auto e_iter = dtsm.cend(); 
  
            for( size_type i = 0; 
                 (i < array_len) && (b_iter != e_iter); 
                 ++b_iter, ++i)
            {
                datetime[i] = b_iter->second.second;

                err = strcpy_s(dest[i], str_len, (b_iter->second.first).as_string().c_str());
                if(err) 
                    return err;   

                if(label_dest){
                    err = strcpy_s(label_dest[i], label_str_len, (b_iter->first).c_str());
                    if(err)              
                        return err;   
                }
            }    
        }else{    
            auto m = db->block->map_of_frame_topics(item);
            auto b_iter = m.cbegin();
            auto e_iter = m.cend();     
 
            for( size_type i = 0; 
                 (i < array_len) && (b_iter != e_iter); 
                 ++b_iter, ++i)
            {
                err = strcpy_s(dest[i], str_len, (b_iter->second).as_string().c_str());
                if(err) 
                    return err;

                if(label_dest){
                    err = strcpy_s(label_dest[i], label_str_len, (b_iter->first).c_str());
                    if(err)
                        return err;  
                }
            }  
        }
        /* --- CRITICAL SECTION --- */
    }catch(const std::exception& e){
        TOSDB_LogH("TOSDB_GetTopicFrameStrings", e.what());
        return -2;
    }catch(...){ 
        return -2; 
    }

  return 0;
}

template<> 
generic_matrix_type 
TOSDB_GetTotalFrame<false>(std::string id)
{
    const TOSDBlock *db;

    GLOBAL_RLOCK_GUARD;
    /* --- CRITICAL SECTION --- */
    db = GetBlockOrThrow(id);
    return db->block->matrix_of_frame();
    /* --- CRITICAL SECTION --- */
}

template<> 
generic_dts_matrix_type 
TOSDB_GetTotalFrame<true>(std::string id)
{
    const TOSDBlock *db;

    GLOBAL_RLOCK_GUARD;
    /* --- CRITICAL SECTION --- */
    db = GetBlockOrThrow(id);
    return db->block->pair_matrix_of_frame();
    /* --- CRITICAL SECTION --- */
}







