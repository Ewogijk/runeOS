#include <stddef.h>

/**
 * @brief Command line arguments and dynamic linker information.
 */
struct StartInfo {
    /**
     * @brief Number of command line arguments.
     */
    int argc;

    /**
     * @brief Null terminated array of command line arguments.
     */
    char** argv;

    /**
     * @brief Low and high bytes of a random 16 byte value.
     */
    unsigned long int random_low;
    unsigned long int random_high;

    /**
     * @brief Virtual address of an array where the ELF program headers are stored.
     */
    void* program_header_address;

    /**
     * @brief Size of a program header.
     */
    size_t program_header_size;

    /**
     * @brief Size of the program header array.
     */
    size_t program_header_count;

    /**
     * @brief Virtual address of the main function.
     */
    void* app_main;

    /**
     * @brief Path to the executable as passed to the kernel.
     */
    const char* executable_name;

    /**
     * @brief Address of a 16 byte random value.
     */
    void* random;
};

extern int main(int argc, char** argv);

extern "C" int forge_main(StartInfo* start_info) {
    return main(start_info->argc, start_info->argv);
}
