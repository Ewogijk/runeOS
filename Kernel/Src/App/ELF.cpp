/*
 *  Copyright 2025 Ewogijk
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 */

#include <App/ELF.h>


namespace Rune::App {
    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
    //                                          Common Definitions
    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//

    IMPLEMENT_TYPED_ENUM(Class, U8, CLASSES, 0)


    IMPLEMENT_TYPED_ENUM(ObjectFileType, U16, OBJECT_FILE_TYPES, 0)


    IMPLEMENT_TYPED_ENUM(SectionType, U32, SECTION_TYPES, 0)


    IMPLEMENT_TYPED_ENUM(SectionAttribute, U32, SECTION_ATTRIBUTES, 0)


    IMPLEMENT_TYPED_ENUM(SegmentType, U32, SEGMENT_TYPES, 0)


    IMPLEMENT_TYPED_ENUM(SegmentPermission, U32, SEGMENT_PERMISSIONS, 0)
}

