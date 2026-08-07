import stdexec;

// CONTROL: in the importer's ordinary context my_tuple<> is complete, and
// this passes. Same type, same TU, same specialization as below.
static_assert(stdexec::my_tuple<>::size == 0);

using fn1 = stdexec::fn;

struct fn2 : stdexec::fn {};

int main() {
  stdexec::__variant<stdexec::my_tuple<>> __args{stdexec::__no_init};

  // visiting with a visitor defined in the module works
  __args.__visit(stdexec::fn{}, __args);

  // visiting with an importer-scoped alias of the module-scoped visitor works
  __args.__visit(fn1{}, __args);

  // defining a type in the importer and visiting with it breaks
  __args.__visit(fn2{}, __args);
}
