struct B {
    void operator()(int a, int b) {
        cout << "a+b: " << a + b << endl;
    }
};

template<typename B>
struct A {
    void operator()(int a, int b) {
        B bm;
        bm(a, b);
    }
};

template<typename A>
void func(int a, int b) {
    A m;
    m(a, b);
}

typedef void(*ResizeFunc)(int, int);

/*
    ResizeFunc f = func<A<B>>;
    f(3,4)->"a+b";
*/

void func_u(const uchar* a) {
    cout << "uchar: " << (int)*a << endl;
}

void func_f(const float* a) {
    cout << "float: " << *a << endl;
}

int func_ff(const float* a) {
    cout << "float with return: " << *a << endl;
    return {};
}

void func_s(const string* s) {
    cout << "s: " << *s << endl;
}

typedef void (*BatchDistFunc)(void*);

void main() {

    void* input;
    float float_val(2.3);
    uchar uchar_val(34);
    string string_val("only you");
    BatchDistFunc func;
    
    func= (BatchDistFunc)func_u; 
    input = (void*)&uchar_val;
    func(input);

    func = (BatchDistFunc)func_f;
    input = (void*)&float_val;
    func(input);
    
    func = (BatchDistFunc)func_s;
    input = (void*)&string_val;
    func(input);

    func = (BatchDistFunc)&func_ff;
    input = (void*)&float_val;
    func(input);//当函数可以忽略返回值时，可以用void擦除返回值类型
}
