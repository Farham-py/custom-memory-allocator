#include <iostream>
#include <cstring>
#include <stdexcept>

using namespace std;

// CONSTANTS

const size_t HEAP_SIZE = 1024 * 1024;
unsigned char HEAP[HEAP_SIZE];

const int MAX_VECTORS = 128;

enum VecType { VEC_INT = 1, VEC_DOUBLE = 2 };


// FUNCTION DECLARATIONS

// control area
size_t& heap_used();
size_t& id_count();
size_t* id_table();
size_t control_area_size();

// header access
size_t header_size();
size_t* h_size(size_t h);
size_t* h_cap(size_t h);
size_t* h_type(size_t h);
size_t* h_block(size_t h);
size_t data_start(size_t h);

// utilities
size_t type_size(size_t t);
void heap_check(size_t bytes);
void validate_id(int id);

// vector core
int vector_create(int type, size_t capacity);
void vector_destroy(int id);
void vector_reserve(int id, size_t new_cap);
void vector_push_back(int id, const void* value);
void* vector_at(int id, size_t index);

// extra operations
void print_vector(int id);
void vector_push_at(int id, size_t index, const void* value);
void vector_pop_back(int id);
void vector_erase_range(int id, size_t start, size_t end);
void vector_clear(int id);
int vector_copy(int id);
void print_memory_status();


// MAIN

int main() {

    // initialize heap control area
    heap_used() = control_area_size();
    id_count() = 0;
    memset(id_table(), 0, MAX_VECTORS * sizeof(size_t));

    try {
        cout <<"CUSTOM VECTOR FULL TEST : \n\n";

        int myVector = vector_create(VEC_INT, 5);
        cout << "Created int vector (myVector)\n";

        int initial_vals[] = {2, 5, 7, 13, 24};
        for (int i = 0; i < 5; ++i)
            vector_push_back(myVector, &initial_vals[i]);

        cout << "After pushing 2,5,7,13,24: ";
        print_vector(myVector);

        int val19 = 19;
        vector_push_at(myVector, 2, &val19);
        cout << "After inserting 19 at index 2: ";
        print_vector(myVector);

        vector_pop_back(myVector);
        cout << "After popping last element: ";
        print_vector(myVector);

        vector_erase_range(myVector, 1, 3);
        cout << "After erasing range index 1 to 3: ";
        print_vector(myVector);

        vector_clear(myVector);
        cout << "After clearing vector: ";
        print_vector(myVector);

        int new_vals[] = {8, 21, 17, 36, 4};
        for (int i = 0; i < 5; ++i)
            vector_push_back(myVector, &new_vals[i]);

        cout << "After adding 8,21,17,36,4: ";
        print_vector(myVector);

        int* third_elem = (int*)vector_at(myVector, 2);
        cout << "3rd element using vector_at: " << *third_elem << "\n";

        int myVector2 = vector_copy(myVector);
        cout << "Copied myVector into myVector2:\n";
        print_vector(myVector2);

        cout << "\nMemory status after copy:\n";
        print_memory_status();

        size_t remaining_bytes = HEAP_SIZE - heap_used();
        size_t double_capacity =
                (remaining_bytes - header_size()) / sizeof(double);

        int myVector3 = vector_create(VEC_DOUBLE, double_capacity);
        cout << "\nCreated myVector3 (double vector) using remaining heap capacity.\n";

        double doubleValue = 1.1;
        for (size_t i = 0; i < double_capacity; ++i)
            vector_push_back(myVector3, &doubleValue);

        cout << "Filled myVector3 completely.\nWhole Heap is full!!!!";

        cout << "\nFinal Heap Status:\n";
        print_memory_status();

        cout << "\nAttempting to push beyond heap capacity...\n";

        try {
            vector_push_back(myVector3, &doubleValue);
        }
        catch (const exception& e) {
            cout << "Exception caught successfully: " << e.what() << "\n";
        }

        cout << "\nHeap status after failed push attempt:\n";
        print_memory_status();

        cout<< "\nDestroying all three vector.....\n";
        vector_destroy(myVector);
        vector_destroy(myVector2);
        vector_destroy(myVector3);

        cout << "\nAll vectors destroyed successfully.\n";
    }
    catch (const exception& e) {
        cerr << "Error: " << e.what() << endl;
    }

    return 0;
}


// FUNCTION DEFINITIONS :

// CONTROL AREA

size_t& heap_used() { return *(size_t*)HEAP; }

size_t& id_count() { return *(size_t*)(HEAP + sizeof(size_t)); }

size_t* id_table() { return (size_t*)(HEAP + 2 * sizeof(size_t)); }

size_t control_area_size() {
    return 2 * sizeof(size_t) + MAX_VECTORS * sizeof(size_t);
}


//HEADER ACCESS

size_t header_size() { return 4 * sizeof(size_t); }

size_t* h_size(size_t h) { return (size_t*)(HEAP + h); }

size_t* h_cap(size_t h) { return (size_t*)(HEAP + h + sizeof(size_t)); }

size_t* h_type(size_t h) { return (size_t*)(HEAP + h + 2 * sizeof(size_t)); }

size_t* h_block(size_t h) { return (size_t*)(HEAP + h + 3 * sizeof(size_t)); }

size_t data_start(size_t h) { return h + header_size(); }


// UTILITIES

size_t type_size(size_t t) {
    if (t == VEC_INT) return sizeof(int);
    if (t == VEC_DOUBLE) return sizeof(double);
    throw runtime_error("Invalid type");
}

void heap_check(size_t bytes) {
    if (heap_used() + bytes > HEAP_SIZE)
        throw runtime_error("Heap overflow");
}

void validate_id(int id) {
    if (id < 0 || id >= (int)id_count())
        throw runtime_error("Invalid ID");
    if (id_table()[id] == 0)
        throw runtime_error("Destroyed vector");
}


// VECTOR CORE

int vector_create(int type, size_t capacity) {
    if (id_count() >= MAX_VECTORS)
        throw runtime_error("Max vectors reached");

    size_t elem = type_size(type);
    size_t total = header_size() + capacity * elem;

    heap_check(total);

    size_t h = heap_used();

    *h_size(h) = 0;
    *h_cap(h) = capacity;
    *h_type(h) = type;
    *h_block(h) = total;

    heap_used() += total;

    id_table()[id_count()] = h;
    return id_count()++;
}

void vector_destroy(int id) {
    validate_id(id);

    size_t h = id_table()[id];
    size_t block = *h_block(h);
    size_t end = h + block;

    memmove(HEAP + h, HEAP + end, heap_used() - end);
    heap_used() -= block;

    for (size_t i = 0; i < id_count(); i++)
        if (id_table()[i] > h)
            id_table()[i] -= block;

    id_table()[id] = 0;
}

void vector_reserve(int id, size_t new_cap) {
    validate_id(id);

    size_t old_h = id_table()[id];
    size_t old_size = *h_size(old_h);
    size_t old_cap = *h_cap(old_h);
    size_t type = *h_type(old_h);

    if (new_cap <= old_cap)
        return;

    size_t elem = type_size(type);
    size_t new_total = header_size() + new_cap * elem;

    heap_check(new_total);

    size_t new_h = heap_used();

    *h_size(new_h) = old_size;
    *h_cap(new_h) = new_cap;
    *h_type(new_h) = type;
    *h_block(new_h) = new_total;

    memcpy(HEAP + data_start(new_h),
           HEAP + data_start(old_h),
           old_size * elem);

    heap_used() += new_total;

    size_t old_block = *h_block(old_h);
    size_t old_end = old_h + old_block;

    memmove(HEAP + old_h, HEAP + old_end, heap_used() - old_end);
    heap_used() -= old_block;

    for (size_t i = 0; i < id_count(); i++)
        if (id_table()[i] > old_h)
            id_table()[i] -= old_block;

    id_table()[id] = new_h - old_block;
}

void vector_push_back(int id, const void* value) {
    validate_id(id);

    size_t h = id_table()[id];
    size_t size = *h_size(h);
    size_t cap = *h_cap(h);
    size_t type = *h_type(h);

    if (size == cap) {
        vector_reserve(id, cap * 2 + 1);
        h = id_table()[id];
    }

    size = *h_size(h);
    size_t elem = type_size(type);

    memcpy(HEAP + data_start(h) + size * elem, value, elem);
    (*h_size(h))++;
}

void* vector_at(int id, size_t index) {
    validate_id(id);

    size_t h = id_table()[id];

    if (index >= *h_size(h))
        throw out_of_range("Index out of range");

    return HEAP + data_start(h)
           + index * type_size(*h_type(h));
}


// EXTRA OPERATIONS

void print_vector(int id) {
    validate_id(id);

    size_t h = id_table()[id];
    size_t size = *h_size(h);
    size_t type = *h_type(h);

    cout << "[ ";

    if (type == VEC_INT) {
        for (size_t i = 0; i < size; ++i)
            cout << *(int*)vector_at(id, i) << " ";
    }
    else if (type == VEC_DOUBLE) {
        for (size_t i = 0; i < size; ++i)
            cout << *(double*)vector_at(id, i) << " ";
    }

    cout << "]\n";
}

void vector_push_at(int id, size_t index, const void* value) {
    validate_id(id);

    size_t h = id_table()[id];
    size_t size = *h_size(h);
    size_t elem = type_size(*h_type(h));

    if (index > size)
        throw out_of_range("Push index out of range");

    if (size == *h_cap(h)) {
        vector_reserve(id, *h_cap(h) * 2 + 1);
        h = id_table()[id];
    }

    size = *h_size(h);

    memmove(HEAP + data_start(h) + (index + 1) * elem,
            HEAP + data_start(h) + index * elem,
            (size - index) * elem);

    memcpy(HEAP + data_start(h) + index * elem, value, elem);
    (*h_size(h))++;
}

void vector_pop_back(int id) {
    validate_id(id);
    if (*h_size(id_table()[id]) == 0)
        throw runtime_error("Pop from empty vector");
    (*h_size(id_table()[id]))--;
}

void vector_erase_range(int id, size_t start, size_t end) {
    validate_id(id);

    size_t h = id_table()[id];
    size_t size = *h_size(h);
    size_t elem = type_size(*h_type(h));

    if (start > end || end >= size)
        throw out_of_range("Invalid erase range");

    size_t count = end - start + 1;

    memmove(HEAP + data_start(h) + start * elem,
            HEAP + data_start(h) + (end + 1) * elem,
            (size - end - 1) * elem);

    *h_size(h) -= count;
}

void vector_clear(int id) {
    validate_id(id);
    *h_size(id_table()[id]) = 0;
}

int vector_copy(int id) {
    validate_id(id);

    size_t h = id_table()[id];
    size_t size = *h_size(h);
    size_t cap = *h_cap(h);
    size_t type = *h_type(h);

    int new_id = vector_create(type, cap);

    memcpy(HEAP + data_start(id_table()[new_id]),
           HEAP + data_start(h),
           size * type_size(type));

    *h_size(id_table()[new_id]) = size;

    return new_id;
}

void print_memory_status() {
    cout << "Heap used: " << heap_used()
         << " bytes\nRemaining: "
         << HEAP_SIZE - heap_used()
         << " bytes\n";
}

