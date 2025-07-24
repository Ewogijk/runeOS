#ifndef ASSERT_H
#define ASSERT_H

#include <LibK/Stream.h>


namespace Rune::LibK {
    /**
     * Configure assert statements to log to the given stream.
     * @param stream Text stream for assertion error messages.
     */
    void assert_configure(const SharedPointer<TextStream>& stream);


    /**
     * Assert that the condition is true, if not log given message and go into an endless loop.
     * @param condition Condition that must be true.
     * @param file      Name of the source file.
     * @param message   Error message to log if !condition.
     */
    void assert(bool condition, const String& file, const String& message);


    /**
     * Assert that the condition is true, if not go into an endless loop.
     * @param condition Condition that must be true.
     * @param file      Name of the source file.
     */
    void assert(bool condition, const String& file);
}

#endif //ASSERT_H
