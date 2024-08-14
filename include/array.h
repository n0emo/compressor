#define array_append(array, item) do { \
    if(array.count == array.capacity) { \
        size_t new_capacity = array.capacity == 0 ? 4 : array.capacity * 2; \
        array.items = realloc(array.items, new_capacity); \
        array.capacity = new_capacity \
    } \
    array.items[array.count] = item; \
    array.count++; \
} while (0);
