// main.cpp
#define __visible__ __attribute__((visibility("default")))

#define NONCLASS_ENTITIES(prefix)                                           \
    /*variables*/                                                           \
    static  __visible__ int prefix ## __variable_static = 0;                \
            __visible__ int prefix ## __variable_noextern = 0;              \
            __visible__ const int prefix ## __variable_noextern_const = 0;  \
    extern  __visible__ int prefix ## __variable_extern;                    \
    int prefix ## __variable_extern = 0;                                    \
                                                                            \
    /*functions*/                                                           \
    __visible__ inline  int prefix ## __function_inline() { return 0; };    \
    __visible__ static  int prefix ## __function_static() { return 0; };    \
    __visible__         int prefix ## __function() { return 0; };           \
    /**/

#define CLASS_MEMBER_ENTITIES(prefix)                                                   \
    public:                                                                             \
        static  int prefix ##   __public_static_member_function_inline() { return 0; }  \
        static  int prefix ##   __public_static_member_function();                      \
        static  int prefix ##   __public_static_member_variable;                        \
                int prefix ##   __public_member_function_inline() { return 0; }         \
        inline  int prefix ##   __public_member_function_inline_not_in_body();          \
                int prefix ##   __public_member_function();                             \
                int prefix ##   __public_member_variable;                               \
    private:                                                                            \
        static  int prefix ##   __private_static_member_function_inline() { return 0; } \
        static  int prefix ##   __private_static_member_function();                     \
        static  int prefix ##   __private_static_member_variable;                       \
                int prefix ##   __private_member_function_inline() { return 0; }        \
                int prefix ##   __private_member_function();                            \
                int prefix ##   __private_member_variable;                              \
    /**/

#define CLASS_MEMBER_ENTITIES_DEFINITION(prefix)                                \
    int prefix ## __public_static_member_function() { return 0; }               \
    int prefix ## __public_static_member_variable = 0;                          \
    int prefix ## __public_member_function() { return 0; }                      \
    int prefix ## __public_member_function_inline_not_in_body() { return 0; }   \
    int prefix ## __private_static_member_function() { return 0; }              \
    int prefix ## __private_static_member_variable = 0;                         \
    int prefix ## __private_member_function() { return 0; }                     \
    /**/

NONCLASS_ENTITIES(global_ns)

struct base_nonvis_class_impl {
    virtual ~base_nonvis_class_impl(){}
    CLASS_MEMBER_ENTITIES(global_ns__base_nonvis_class)
};
CLASS_MEMBER_ENTITIES_DEFINITION(base_nonvis_class_impl::global_ns__base_nonvis_class)


struct __visible__ some_class: base_nonvis_class_impl {
    CLASS_MEMBER_ENTITIES(global_ns__some_class)

    int foo() {
        return base_nonvis_class_impl::global_ns__base_nonvis_class__public_member_function();
    }
public:
    struct public_nested_class { 
        CLASS_MEMBER_ENTITIES(global_ns__some_class__public_nested_class)
    };

    struct public_nested_class_not_inside;
private:
    struct private_nested_class {
        CLASS_MEMBER_ENTITIES(global_ns__some_class__private_nested_class)
    };
};

struct  some_class::public_nested_class_not_inside {
    CLASS_MEMBER_ENTITIES(global_ns__some_class__public_nested_class_not_inside)
};

CLASS_MEMBER_ENTITIES_DEFINITION(some_class::global_ns__some_class)
CLASS_MEMBER_ENTITIES_DEFINITION(some_class::public_nested_class::global_ns__some_class__public_nested_class)
CLASS_MEMBER_ENTITIES_DEFINITION(some_class::private_nested_class::global_ns__some_class__private_nested_class)
CLASS_MEMBER_ENTITIES_DEFINITION(some_class::public_nested_class_not_inside::global_ns__some_class__public_nested_class_not_inside)


struct derived_nonvis_class_impl: some_class {
    CLASS_MEMBER_ENTITIES(global_ns__derived_nonvis_class)
};
CLASS_MEMBER_ENTITIES_DEFINITION(derived_nonvis_class_impl::global_ns__derived_nonvis_class)


namespace /*anonymous_ns*/ {
    NONCLASS_ENTITIES(anonymous_ns)
    struct __visible__ anonymous_ns_class {
        CLASS_MEMBER_ENTITIES(anonymous_ns__some_class)
    };
    CLASS_MEMBER_ENTITIES_DEFINITION(anonymous_ns_class::anonymous_ns__some_class)
}

namespace visible_ns __visible__ {
    NONCLASS_ENTITIES(visible_ns)
    struct __visible__ visible_ns_class {
        CLASS_MEMBER_ENTITIES(visible_ns__some_class)
    };
    CLASS_MEMBER_ENTITIES_DEFINITION(visible_ns_class::visible_ns__some_class)

    namespace nested {
        NONCLASS_ENTITIES(visible_ns__nested_ns)
        struct __visible__ visible_ns_nested_ns_class {
            CLASS_MEMBER_ENTITIES(visible_ns__nested_ns__some_class)
        public:
            struct public_nested_class { 
                CLASS_MEMBER_ENTITIES(visible_ns__nested_ns__some_class__public_nested_class)
            };

            struct public_nested_class_not_inside;
        };

        struct visible_ns_nested_ns_class::public_nested_class_not_inside {
                CLASS_MEMBER_ENTITIES(visible_ns__nested_ns__some_class__public_nested_class_not_inside)
        };

        CLASS_MEMBER_ENTITIES_DEFINITION(visible_ns_nested_ns_class::visible_ns__nested_ns__some_class)
        CLASS_MEMBER_ENTITIES_DEFINITION(visible_ns_nested_ns_class::public_nested_class::visible_ns__nested_ns__some_class__public_nested_class)
        CLASS_MEMBER_ENTITIES_DEFINITION(visible_ns_nested_ns_class::public_nested_class_not_inside::visible_ns__nested_ns__some_class__public_nested_class_not_inside)
    }
}

