#define __visible__ __attribute__((visibility("default")))

static __visible__ int cpp_variable_static = 0;
__visible__ int cpp_variable_noextern = 0;
__visible__ const int cpp_variable_noextern_const = 0;

extern __visible__ int cpp_variable_extern;
int cpp_variable_extern = 0;

__visible__ inline int cpp_function_inline() { return 0; };
__visible__ static int cpp_function_static() { return 0; };
__visible__ int cpp_function() { return 0; };



struct base_nonvis_class_impl {
    static int cpp_base_nonvis_static_member_function_inline(){ return 0; }
    static int cpp_base_nonvis_static_member_function();
    static int cpp_base_nonvis_static_member_variable;

    int cpp_base_nonvis_member_function_inline(){ return 0; }
    int cpp_base_nonvis_member_function();
    int cpp_base_nonvis_member_variable;

    virtual ~base_nonvis_class_impl(){}

private:
    static int cpp_base_nonvis_private_static_member_function_inline(){ return 0; }
    static int cpp_base_nonvis_private_static_member_function();
    static int cpp_base_nonvis_private_static_member_variable;

    int cpp_base_nonvis_private_member_function_inline(){ return 0; }
    int cpp_base_nonvis_private_member_function();
    int cpp_base_nonvis_private_member_variable;
};
int base_nonvis_class_impl::cpp_base_nonvis_static_member_variable = 0;
int base_nonvis_class_impl::cpp_base_nonvis_static_member_function() { return 0; }
int base_nonvis_class_impl::cpp_base_nonvis_member_function() { return 0; }

int base_nonvis_class_impl::cpp_base_nonvis_private_static_member_variable = 0;
int base_nonvis_class_impl::cpp_base_nonvis_private_static_member_function() { return 0; }
int base_nonvis_class_impl::cpp_base_nonvis_private_member_function() { return 0; }



struct __visible__ class_impl: base_nonvis_class_impl {
    static int cpp_static_member_function_inline(){ return 0; }
    static int cpp_static_member_function();
    static int cpp_static_member_variable;

    int cpp_member_function_inline(){ return 0; }
    int cpp_member_function();
    int cpp_member_variable;

private:
    static int cpp_private_static_member_function_inline(){ return 0; }
    static int cpp_private_static_member_function();
    static int cpp_private_static_member_variable;

    int cpp_private_member_function_inline(){ return 0; }
    int cpp_private_member_function();
    int cpp_private_member_variable;
};
int class_impl::cpp_static_member_variable = 0;
int class_impl::cpp_static_member_function() { return 0; }
int class_impl::cpp_member_function() { return 0; }

int class_impl::cpp_private_static_member_variable = 0;
int class_impl::cpp_private_static_member_function() { return 0; }
int class_impl::cpp_private_member_function() { return 0; }



struct derived_nonvis_class_impl: class_impl {
    static int cpp_derived_nonvis_static_member_function_inline(){ return 0; }
    static int cpp_derived_nonvis_static_member_function();
    static int cpp_derived_nonvis_static_member_variable;

    int cpp_derived_nonvis_member_function_inline(){ return 0; }
    int cpp_derived_nonvis_member_function();
    int cpp_derived_nonvis_member_variable;

private:
    static int cpp_derived_nonvis_private_static_member_function_inline(){ return 0; }
    static int cpp_derived_nonvis_private_static_member_function();
    static int cpp_derived_nonvis_private_static_member_variable;

    int cpp_derived_nonvis_private_member_function_inline(){ return 0; }
    int cpp_derived_nonvis_private_member_function();
    int cpp_derived_nonvis_private_member_variable;
};
int derived_nonvis_class_impl::cpp_derived_nonvis_static_member_variable = 0;
int derived_nonvis_class_impl::cpp_derived_nonvis_static_member_function() { return 0; }
int derived_nonvis_class_impl::cpp_derived_nonvis_member_function() { return 0; }

int derived_nonvis_class_impl::cpp_derived_nonvis_private_static_member_variable = 0;
int derived_nonvis_class_impl::cpp_derived_nonvis_private_static_member_function() { return 0; }
int derived_nonvis_class_impl::cpp_derived_nonvis_private_member_function() { return 0; }

namespace /*anonymous_ns*/ {

static __visible__ int cpp_anonymous_ns_variable_static = 0;
__visible__ int cpp_anonymous_ns_variable_noextern = 0;
extern __visible__ int cpp_anonymous_ns_variable_extern;
__visible__ inline int cpp_anonymous_ns_function_inline() { return 0; };
__visible__ static int cpp_anonymous_ns_function_static() { return 0; };
__visible__ int cpp_anonymous_ns_function() { return 0; };

struct __visible__ class_impl {
    static int cpp_anonymous_ns_static_member_function_inline(){ return 0; }
    static int cpp_anonymous_ns_static_member_function();
    static int cpp_anonymous_ns_static_member_variable;

    int cpp_anonymous_ns_member_function_inline(){ return 0; }
    int cpp_anonymous_ns_member_function();
    int cpp_anonymous_ns_member_variable;
};

int cpp_anonymous_ns_variable_extern = 0;
int class_impl::cpp_anonymous_ns_static_member_variable = 0;
int class_impl::cpp_anonymous_ns_static_member_function() { return 0; }
int class_impl::cpp_anonymous_ns_member_function() { return 0; }

}
