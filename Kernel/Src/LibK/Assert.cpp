#include <LibK/Assert.h>


namespace Rune::LibK {
    auto err_stream = SharedPointer<TextStream>();


    void assert_configure(const SharedPointer<LibK::TextStream>& stream) {
        if (err_stream) return;
        err_stream = stream;
    }


    void assert(const bool condition, const String& file, const String& message) {
        if (condition) return;
        if (err_stream) err_stream->write(file + " - " + message);
        FOR_ETERNITY()
    }


    void assert(const bool condition, const String& file) {
        if (condition) return;
        if (err_stream) err_stream->write(file + " - Assertion failed!");
        FOR_ETERNITY()
    }
}
