
#ifndef DATA_DEFINITION_H
#define DATA_DEFINITION_H

#ifdef __cplusplus
extern "C"
{
#endif // __cplusplus

#define DATA_CREATE (void *(*)())

#define DATA_DESTROY (void (*)(void *))

#define DATA_COPY (void (*)(void *, const void *))

#define DATA_COMPARE (int (*)(const void *, const void *))

    typedef struct data_op
    {
        /********************************************
         * # alloc memory for data
         * @_data: another data
         * @return: new data
         *********************************************/
        void *(*create)();

        /********************************************
         * # data destructor
         * @_data: data to be destruct
         * @return: 0 is success
         *********************************************/
        void (*destroy)(void *_data);

        /********************************************
         * # copy data from source to destination
         * @_dest: data destination
         * @_src: data source
         * @return: size of data
         *********************************************/
        void (*copy)(void *_dest, const void *_src);

        /********************************************
         * # compare two data objects
         * @_a: data object A
         * @_b: data object B
         * @return: 0 is A==B, 1 is A > B, -1 is A < B
         *********************************************/
        int (*cmp)(const void *_a, const void *_b);
    } data_op;

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // DATA_DEFINITION_H
