//
//  DAFImplicitHeap.h
//  LightsOut
//
//  Created by David Flores on 8/30/15.
//  Copyright (c) 2015 David Flores. All rights reserved.
//

#ifndef DAFImplicitHeap_h
#define DAFImplicitHeap_h

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
namespace DAF
{
    template <typename Key, typename Value>
    class ImplicitHeap
    {
    public:
        ImplicitHeap(std::size_t uMaxNumberOfElements);
        
        void Insert(Key k, Value v);
        std::vector<Value> Values();
        
    private:
        void HeapifyUp();
        void HeapifyDown();
        
        struct Entry
        {
            Key k;
            Value v;
        };
        
        const std::size_t _kuMaxNumberOfElements;
        std::vector<Entry> _vHeap;
    };

    template <typename Key, typename Value>
    ImplicitHeap<Key, Value>::ImplicitHeap(std::size_t uMaxNumberOfElements) :
        _kuMaxNumberOfElements(uMaxNumberOfElements)
    {
        assert( uMaxNumberOfElements > 0 );
        
        _vHeap.reserve(uMaxNumberOfElements + 1);
        
        Entry entry;
        _vHeap.push_back(entry);
    }
    
    template <typename Key, typename Value>
    void
    ImplicitHeap<Key, Value>::Insert(Key k, Value v)
    {
        if ( _vHeap.size() <= (_kuMaxNumberOfElements + 1) )
        {
            Entry entry =
            {
                .k = k,
                .v = v
            };
            
            _vHeap.push_back(entry);
            
            HeapifyUp();
            
            return;
        }
        
        if ( k <= _vHeap[1].k )
            return;
        
        Entry entry =
        {
            .k = k,
            .v = v
        };
        
        _vHeap[1] = entry;
        
        HeapifyDown();
    }
    
    template <typename Key, typename Value>
    std::vector<Value>
    ImplicitHeap<Key, Value>::Values()
    {
        std::vector<Value> vValue;
        vValue.reserve(_vHeap.size() - 1);
        
        for (std::size_t uIndex = 1; uIndex < _vHeap.size(); ++uIndex)
            vValue.push_back(_vHeap[uIndex].v);
        
        return vValue;
    }
    
    template <typename Key, typename Value>
    void
    ImplicitHeap<Key, Value>::HeapifyUp()
    {
        std::size_t uIndex = _vHeap.size() - 1;
        
        for (std::size_t uParentIndex = uIndex / 2;
             (uParentIndex > 0) &&
             (_vHeap[uIndex].k < _vHeap[uParentIndex].k);
             uParentIndex = uParentIndex / 2)
        {
            Entry entryTemp = _vHeap[uParentIndex];
            _vHeap[uParentIndex] = _vHeap[uIndex];
            _vHeap[uIndex] = entryTemp;
            
            uIndex = uParentIndex;
        }
    }
    
    template <typename Key, typename Value>
    void
    ImplicitHeap<Key, Value>::HeapifyDown()
    {
        std::size_t uIndex = 1;
        
        for(;;)
        {
            std::size_t uLeftChildIndex = uIndex * 2;
            std::size_t uRightChildIndex = uLeftChildIndex + 1;
            
            if ( uLeftChildIndex >= _vHeap.size() )
                return;
            
            std::size_t uSmallestChildIndex =
                ( (uRightChildIndex < _vHeap.size()) &&
                  (_vHeap[uRightChildIndex].k < _vHeap[uLeftChildIndex].k) ) ?
                    uRightChildIndex :
                    uLeftChildIndex;
            
            if ( _vHeap[uIndex].k > _vHeap[uSmallestChildIndex].k )
            {
                Entry entryTemp = _vHeap[uSmallestChildIndex];
                _vHeap[uSmallestChildIndex] = _vHeap[uIndex];
                _vHeap[uIndex] = entryTemp;
                
                uIndex = uSmallestChildIndex;
            }
            else
            {
                return;
            }
        }
    }
};

#endif
