# Read the .def file
file(STRINGS ${INPUT_DEF_FILE} NVE_OBJECT_TYPE_LINES
    REGEX "^NVE_REGISTER_TYPE\\([A-Za-z_][A-Za-z0-9_]*\\)$")

# Write the header
file(WRITE ${OUTPUT_CPP_FILE} "#include \"${HEADER_NAME}\"\n\n")

foreach(_line IN LISTS NVE_OBJECT_TYPE_LINES)
    string(REGEX REPLACE "^NVE_REGISTER_TYPE\\(([A-Za-z_][A-Za-z0-9_]*)\\)$" "\\1" _type ${_line})
    file(APPEND ${OUTPUT_CPP_FILE} "#include \"${_type}.h\"\n")
endforeach()

# Write the middle structure
file(APPEND ${OUTPUT_CPP_FILE}
    "\n#include \"model/ObjectRegistry.h\"\n\nnamespace ${NAMESPACE_NAME}\n{\n    void ${FUNCTION_NAME}()\n    {\n")

foreach(_line IN LISTS NVE_OBJECT_TYPE_LINES)
    string(REGEX REPLACE "^NVE_REGISTER_TYPE\\(([A-Za-z_][A-Za-z0-9_]*)\\)$" "\\1" _type ${_line})
    # Explicitly scoping nuvelocity::ObjectRegistry in case the namespace is deeply nested
    file(APPEND ${OUTPUT_CPP_FILE}
        "        nuvelocity::ObjectRegistry::Get().Register(${_type}::GetClassInfo());\n")
endforeach()

# Write the footer
file(APPEND ${OUTPUT_CPP_FILE} "    }\n} // namespace ${NAMESPACE_NAME}\n")