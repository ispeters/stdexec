import stdexec;

// CONTROL: in the importer's ordinary context my_tuple<> is complete, and
// this passes. Same type, same TU, same specialization as below.
static_assert(stdexec::my_tuple<>::size == 0);

using fn1 = stdexec::fn;

struct fn2 : stdexec::fn {};

struct fn3 : stdexec::fn {
  void operator()(auto) {
    stdexec::fn::operator()(stdexec::my_tuple<>{});
  }
};

struct fn3_5 : stdexec::fn {
  void operator()(auto __t) {
    stdexec::fn::operator()(stdexec::my_tuple<>(__t));
  }
};

struct fn4 {
  void operator()(stdexec::my_tuple<> __t) {
    static_assert(stdexec::my_tuple<>::size == 0);
  }
};

int main() {
  stdexec::__variant<stdexec::my_tuple<>> __args{stdexec::__no_init};

  // visiting with a visitor defined in the module works
  __args.__visit(stdexec::fn{}, __args);

  // visiting with an importer-scoped alias of the module-scoped visitor works
  __args.__visit(fn1{}, __args);

  // defining a type in the importer and visiting with it breaks
  __args.__visit(fn2{}, __args);

  // it works to implement a delegation from the importer's member function the module's
  __args.__visit(fn3{}, __args);

  // breaks with "invalid functional cast"
  __args.__visit(fn3_5{}, __args);

  // also broken when the importer's type doesn't inherit from the module's type
  __args.__visit(fn4{}, __args);
}
