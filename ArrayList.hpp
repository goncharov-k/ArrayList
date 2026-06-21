#include <cstddef>
#include <utility> 
#include <algorithm>
#include <new>
#include <stdexcept>
#include <iterator>
#include <initializer_list>    

template<typename T>
class ArrayList {
public:
    using value_type = T;
    using size_type = std::size_t;
    using difference_type = std::ptrdiff_t;
    using reference = T&;
    using const_reference = const T&;
    using pointer = T*;
    using const_pointer = const T*;

    class iterator;
    class const_iterator;
    using reverse_iterator = std::reverse_iterator<iterator>;
    using const_reverse_iterator = std::reverse_iterator<const_iterator>;

private:
    pointer start;
    pointer finish;
    pointer current;

public:

    ArrayList() : start(nullptr), finish(nullptr), current(nullptr) {}

    ArrayList(std::initializer_list<T> list) {
        size_type size = list.size();
        start = static_cast<pointer>(::operator new(size * sizeof(value_type)));
        finish = start + size;
        current = start;

        for (auto i : list) {
            new (current) value_type(i);
            current++;
        }
    }
    ArrayList(size_type count) {
        start = static_cast<pointer>(::operator new(count * sizeof(value_type)));
        finish = start + count;
        current = start;
        while (current != finish) {
            new (current) value_type();
            ++current;
        }
    }

    ArrayList(size_type count, const_reference value) {
        start = static_cast<pointer>(::operator new(count * sizeof(value_type)));
        finish = start + count;
        current = start;
        while (current != finish){
            new (current) value_type(value);
            ++current;
        }
    }

    template<typename InputIt,typename = std::enable_if_t<!std::is_integral<InputIt>::value>>
    ArrayList(InputIt first, InputIt last) {
        size_type count = last - first;
        start = static_cast<pointer>(::operator new(count * sizeof(value_type)));
        finish = start + count;
        current = start;
        while (first != last) {
            new (current) value_type(*first);
            ++current;
            ++first;
        }
    }
    ArrayList(const ArrayList& list) {
        size_type size = list.size();
        start = static_cast<pointer>(::operator new(size * sizeof(value_type)));
        finish = start + size;
        current = finish;

        for (size_type i = 0; i < size; ++i)
            new (start + i) value_type(list[i]);
    }

    ArrayList(ArrayList&& list) noexcept {
        start = list.start;
        finish = list.finish;
        current = list.current;

        list.start = nullptr;
        list.finish = nullptr;
        list.current = nullptr;
    }

    ~ArrayList() {
        if (start != nullptr) {
            for (size_type i = 0; i < (current - start); ++i)
                (start + i)->~value_type();
            ::operator delete(start);
        }
    }

    ArrayList& operator=(const ArrayList& right) {
        if (this == &right)
            return *this;

        size_type size = right.size();
        size_type this_size = current - start;

        if (capacity() < size) {
            if (start != nullptr) {
                for (size_type i = 0; i < this_size; ++i)
                    (start + i)->~value_type();
                ::operator delete(start);
            }

            start = static_cast<pointer>(::operator new(size * sizeof(value_type)));
            finish = start + size;

            for (size_type i = 0; i < size; ++i)
                new (start + i) value_type(right[i]);
        }
        else {
            if (this_size >= size) {
                for (size_type i = 0; i < size; ++i)
                    *(start + i) = right[i];

                for (size_type i = size; i < this_size; ++i)
                    (start + i)->~value_type();
            }
            else {
                for (size_type i = 0; i < this_size; ++i)
                    *(start + i) = right[i];

                for (size_type i = this_size; i < size; ++i)
                    new (start + i) value_type(right[i]);
            }
        }

        current = start + size;
        return *this;
    }

    ArrayList& operator=(ArrayList&& right) noexcept {
        if (this == &right)
            return *this;

        if (start != nullptr) {
            for (size_type i = 0; i < (current - start); ++i)
                (start + i)->~value_type();
            ::operator delete(start);
        }

        start = right.start;
        finish = right.finish;
        current = right.current;

        right.start = nullptr;
        right.finish = nullptr;
        right.current = nullptr;

        return *this;
    }

    ArrayList& operator=(std::initializer_list<T> list) {
        assign(list.begin(), list.end());
        return *this;
    }
    size_type size()     const noexcept { return start ? (current - start) : 0; }
    size_type capacity() const noexcept { return start ? (finish - start) : 0; }

    pointer   data() noexcept { return start; }
    const_pointer data() const noexcept { return start; }

    reference front() { return *start; }
    const_reference front() const { return *start; }
    reference back() { return *(current - 1); }
    const_reference back() const { return *(current - 1); }


    size_type max_size() const noexcept { return size_type(-1) / sizeof(value_type); }



    bool empty() const { return start == current; }

    reference       operator[](size_type index) { return *(start + index); }
    const_reference operator[](size_type index) const { return *(start + index); }

    reference at(size_type index) {
        if (index >= size())
            throw std::out_of_range();
        return *(start + index);
    }

    const_reference at(size_type index) const {
        if (index >= size())
            throw std::out_of_range();
        return *(start + index);
    }

    void add_capacity(size_type size) {
        if (size == 0)return;

        if (start == nullptr) {
            start = static_cast<pointer>(::operator new(size * sizeof(value_type)));
            finish = start + size;
            current = start;
        }
        else {
            size_type old_size = current - start;
            size_type new_cap = (finish - start) + size;

            pointer new_start = static_cast<pointer>(::operator new(new_cap * sizeof(value_type)));

            for (size_type i = 0; i < old_size; ++i)
                new (new_start + i) value_type(std::move_if_noexcept(*(start + i)));

            for (size_type i = 0; i < old_size; ++i)
                (start + i)->~value_type();
            ::operator delete(start);

            start = new_start;
            finish = new_start + new_cap;
            current = new_start + old_size;
        }
    }

    void push_back(const_reference value) {
        if (current == finish)
            add_capacity(start ? finish - start : 1);

        new (current) value_type(value);
        ++current;
    }

    void push_back(value_type&& value) {
        if (current == finish)
            add_capacity(start ? finish - start : 1);

        new (current) value_type(std::move(value));
        ++current;
    }

    void push_back(size_type count, const_reference value) {
        pointer new_current = current + count;
        if (new_current > finish)
        {
            size_type cap = start ? (finish - start) : count;

            while (new_current > finish + cap)
                cap <<= 1;

            add_capacity(cap);
            new_current = current + count;
        }

        while (current != new_current) {
            new (current) value_type(value);
            ++current;
        }
    }
    template<typename InputIt, typename = std::enable_if_t<!std::is_integral<InputIt>::value>>
    void push_back(InputIt first, InputIt last){
        size_type count = last - first;
        pointer new_current = current + count;
        if (new_current > finish)
        {
            size_type cap = start ? (finish - start) : count;
            
            while (new_current > finish + cap)
                cap <<= 1;

            add_capacity(cap);
            new_current = current + count;
        }
        while (current != new_current) {
            new (current) value_type(*first);
            ++current;
            ++first;
        }

    }
    void resize(size_type new_size) {
        size_type old_size = size();
        if (new_size <= old_size) 
            while (current != start + new_size) {
                --current;
                current-> ~value_type();
            }
        else {
            if(current == finish)
              add_capacity(new_size - old_size);

            while (current != start + new_size) {
                new (current) value_type();
                ++current;
            }
        }
    } 

    void resize(size_type new_size,const_reference value) {
        size_type old_size = size();
        if (new_size <= old_size)
            while (current != start + new_size) {
                --current;
                current-> ~value_type();
            }
        else {
            if (current == finish)
                add_capacity(new_size - old_size);

            while (current != start + new_size) {
                new (current) value_type(value);
                ++current;
            }
        }
    }
    void reserve(size_type new_cap) {
        size_type old_cap = capacity();
        if (new_cap <= old_cap) return;
        add_capacity(new_cap - old_cap);
    }

    void assign(size_type count, const_reference value) {
        clear();
        size_type cap = finish - start;
        if (cap < count)
            add_capacity(count - cap);
        while (current != start + count) {
            new (current) value_type(value);
            ++current;
        }
    }

    template<typename InputIt, typename = std::enable_if_t<!std::is_integral<InputIt>::value>>
    void assign(InputIt first, InputIt last) {
        size_type count = last - first;
        size_type cap = finish - start;
        clear();
        if (cap < count)
            add_capacity(count - cap);
        for (size_type i = 0; i < count; ++i){
            new (current) value_type(*first);
            ++first;
            ++current;
        }
    }
    void assign(std::initializer_list<T> list) {
        assign(list.begin(), list.end());
    }
    void pop_back() {
        --current;
        current->~value_type();
    }

    void pop_back(size_type count) {
        for (size_type i = 0; i < count; ++i) {
            --current;
            current->~value_type();
        }
    }

    void clear() {
        while (current != start) {
            --current;
            (current)->~value_type();
        }
    }
    void shrink_to_fit() {
        if (current == finish) return;

        size_type new_cap = size();
        pointer new_start = static_cast<pointer>(::operator new(new_cap * sizeof(value_type)));

        for (size_type i = 0; i < new_cap; ++i)
            new (new_start + i) value_type(std::move_if_noexcept(*(start + i)));

        for (size_type i = 0; i < new_cap; ++i)
            (start + i)->~value_type();
        ::operator delete(start);
        start = new_start;
        finish = new_start + new_cap;
        current = finish;
    }

    template<typename... Args>
    void emplace_back(Args&&... args) {
        if (current == finish)
            add_capacity(start ? finish - start : 1);
        new (current) value_type(std::forward<Args>(args)...);
        ++current;
    }

    void swap(ArrayList& other) noexcept {
            std::swap(start, other.start);
            std::swap(finish, other.finish);
            std::swap(current, other.current);
        }

    bool operator==(const ArrayList& other) const noexcept {
        size_type size_ = size();
        if (size_ != other.size())
            return false;
        for (size_type i = 0; i < size_; ++i)
            if (start[i] != other[i]) return false;
        return true;
    }
    bool operator!=(const ArrayList& other)const noexcept {
        return !operator==(other);
    }
    friend bool operator<(const ArrayList& left, const ArrayList& right) noexcept {
        return std::lexicographical_compare(left.begin(), left.end(), right.begin(), right.end());
    }
    friend bool operator>(const ArrayList& left, const ArrayList& right) noexcept { return right < left; }
    friend bool operator<=(const ArrayList& left, const ArrayList& right) noexcept { return !(right < left); }
    friend bool operator>=(const ArrayList& left, const ArrayList& right) noexcept { return !(left < right); }

    
    class iterator {
    public:
        using value_type = T;
        using difference_type = std::ptrdiff_t;
        using pointer = T*;
        using reference = T&;
        using iterator_category = std::random_access_iterator_tag;
    private:
        pointer ptr;

    public:
        iterator() : ptr(nullptr) {}
        iterator(pointer p) : ptr(p) {}
        

        reference operator*() const { return *ptr; }
        pointer   operator->() const { return ptr; }
        reference operator[](difference_type n) const { return ptr[n]; }

        iterator& operator++()  { ++ptr; return *this; }
        iterator  operator++(int) {
            iterator result = *this;
            ++ptr;
            return result;
        }

        iterator& operator--() { --ptr; return *this; }
        iterator  operator--(int) {
            iterator result = *this;
            --ptr;
            return result;
        }

        iterator& operator+=(difference_type right) { ptr += right; return *this; }
        iterator& operator-=(difference_type right) { ptr -= right; return *this; }

        friend iterator operator+(iterator left, difference_type right) { return left += right; }
        friend iterator operator+(difference_type left, iterator right) { return right += left; }
        friend iterator operator-(iterator left, difference_type right) { return left -= right; }
        friend difference_type operator-(const iterator& left, const iterator& right) { return left.ptr - right.ptr; }

        bool operator==(const iterator& right) const { return ptr == right.ptr; }
        bool operator!=(const iterator& right) const { return ptr != right.ptr; }
        bool operator> (const iterator& right) const { return ptr > right.ptr; }
        bool operator< (const iterator& right) const { return ptr < right.ptr; }
        bool operator>=(const iterator& right) const { return ptr >= right.ptr; }
        bool operator<=(const iterator& right) const { return ptr <= right.ptr; }
    };

    class const_iterator {
    public:
        using value_type = T;
        using difference_type = std::ptrdiff_t;
        using pointer = const T*;
        using reference = const T&;
        using iterator_category = std::random_access_iterator_tag;

    private:
        pointer ptr;

    public:
        const_iterator() : ptr(nullptr) {}
        const_iterator(pointer p) : ptr(p) {}
        const_iterator(const iterator& it) : ptr(it.operator->()) {}

        reference operator*() const { return *ptr; }
        pointer   operator->() const { return ptr; }
        reference operator[](difference_type n) const { return ptr[n]; }

        const_iterator& operator++() { ++ptr; return *this; }
        const_iterator  operator++(int) {
            const_iterator result = *this;
            ++ptr;
            return result;
        }

        const_iterator& operator--() { --ptr; return *this; }
        const_iterator  operator--(int) {
            const_iterator result = *this;
            --ptr;
            return result;
        }

        const_iterator& operator+=(difference_type right) { ptr += right; return *this; }
        const_iterator& operator-=(difference_type right) { ptr -= right; return *this; }

        friend const_iterator operator+(const_iterator left, difference_type right) { return left += right; }
        friend const_iterator operator+(difference_type left, const_iterator right) { return right += left; }
        friend const_iterator operator-(const_iterator left, difference_type right) { return left -= right; }
        friend difference_type operator-(const const_iterator& left, const const_iterator& right) { return left.ptr - right.ptr; }

        bool operator==(const const_iterator& right) const { return ptr == right.ptr; }
        bool operator!=(const const_iterator& right) const { return ptr != right.ptr; }
        bool operator> (const const_iterator& right) const { return ptr > right.ptr; }
        bool operator< (const const_iterator& right) const { return ptr < right.ptr; }
        bool operator>=(const const_iterator& right) const { return ptr >= right.ptr; }
        bool operator<=(const const_iterator& right) const { return ptr <= right.ptr; }


        friend bool operator==(const const_iterator& left, const iterator& right) {
            return left.ptr == right.operator->();
        }
        friend bool operator==(const iterator& left, const const_iterator& right) {
            return right == left;
        }
        friend bool operator< (const const_iterator& left, const iterator& right) {
            return left.operator->() < right.operator->();
        }
        friend bool operator< (const iterator& left, const const_iterator& right) {
            return left.operator->() < right.operator->();
        }
        friend bool operator> (const const_iterator& left, const iterator& right) {
            return left.operator->() > right.operator->();
        }
        friend bool operator> (const iterator& left, const const_iterator& right) {
            return left.operator->() > right.operator->();
        }
        friend bool operator<=(const const_iterator& left, const iterator& right) {
            return left.operator->() <= right.operator->();
        }
        friend bool operator<=(const iterator& left, const const_iterator& right) {
            return left.operator->() <= right.operator->();
        }
        friend bool operator>=(const const_iterator& left, const iterator& right) {
            return left.operator->() >= right.operator->();
        }
        friend bool operator>=(const iterator& left, const const_iterator& right) {
            return left.operator->() >= right.operator->();
        }
       
    };
    

    iterator begin() { return iterator(start); }
    iterator end() { return iterator(current); }

    const_iterator begin() const { return const_iterator(start); }
    const_iterator end()   const { return const_iterator(current); }

    const_iterator cbegin() const { return begin(); }
    const_iterator cend()   const { return end(); }
    
    reverse_iterator rbegin() { return reverse_iterator(end()); }
    reverse_iterator rend() { return reverse_iterator(begin()); }

    const_reverse_iterator rbegin() const { return const_reverse_iterator(end()); }
    const_reverse_iterator rend()   const { return const_reverse_iterator(begin()); }

    const_reverse_iterator crbegin() const { return rbegin(); }
    const_reverse_iterator crend()   const { return rend(); }

    iterator insert(const_iterator pos, const_reference value) {
        if (pos == end()) {
            push_back(value);
            return iterator(current - 1);
        }

        size_type index = pos - cbegin();
        push_back(value);
        
        pointer ptr = start + index;

        pointer i = current - 1;
        while (i != ptr) {
            std::swap(*i, *(i - 1));
            --i;
        }
        return iterator(ptr);
    }

    iterator insert(const_iterator pos, value_type&& value) {
        size_type index = pos - cbegin();
        if (pos == end()) {
            push_back(std::move(value));
            return iterator(current - 1);
        }

        
        push_back(std::move(value));
        
        pointer ptr = start + index;

        pointer i = current - 1;
        while (i != ptr) {
            std::swap(*i, *(i - 1));
            --i;
        }
        return iterator(ptr);
    }
    iterator insert(const_iterator pos, size_type count, const_reference value) {
        size_type index = pos - cbegin();

        if (pos == end()) {
            push_back(count, value);
            return iterator(start + index);
        }

        
        push_back( count, value);

        pointer ptr = start + index + count;

        pointer i = current - 1;
        while (i >= ptr) {
            std::swap(*i, *(i - count));
            --i;
        }

        return iterator(start + index);
    }

    template<typename InputIt, typename = std::enable_if_t<!std::is_integral<InputIt>::value>>
    iterator insert(const_iterator pos, InputIt first, InputIt last) {
        size_type index = pos - cbegin();

        if (pos == end()) {
            push_back(first, last);
            return iterator(start + index);
        }
        size_type count =  last - first;

        push_back(first,last);

        pointer ptr = start + index + count;

        pointer i = current - 1;
        while (i >= ptr) {
            std::swap(*i, *(i - count));
            --i;
        }

        return iterator(start + index);
    }

    iterator erase(const_iterator pos) {
        pointer pos_ptr = start + (pos - cbegin());
        

        pointer ptr = pos_ptr;

        while (ptr != current - 1) {
            std::swap(*ptr, *(ptr + 1));
            ++ptr;
        }
        pop_back();
        return iterator(pos_ptr);
    }

   iterator erase(const_iterator first,const_iterator last) {
       size_type count = last - first;
       pointer first_ptr = start + (first - cbegin());
        
       if (last == end()) {
            pop_back(count);
            return iterator(first_ptr);
        }

        pointer ptr = first_ptr;
        while (ptr != current - count) {
            std::swap(*ptr, *(ptr + count));
                ++ptr;
        }
        pop_back(count);
        return iterator(first_ptr);
    }

   template<typename... Args>
   iterator emplace(const_iterator pos, Args&&... args) {
       if (pos == end())
       {
           emplace_back(std::forward<Args>(args)...);
           return iterator(current - 1);
       }
       size_type index = pos - cbegin();
       emplace_back(std::forward<Args>(args)...);

       pointer ptr = start + index;

       pointer i = current - 1;
       while (i != ptr) {
           std::swap(*i, *(i - 1));
           --i;
       }
       return iterator(ptr);
   }

   friend void swap(ArrayList& a, ArrayList& b) noexcept {
       a.swap(b);
   }
};



