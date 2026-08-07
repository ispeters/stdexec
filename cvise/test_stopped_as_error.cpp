import foo;
struct ifn { void operator()(bar::box<>) {} };   // importer-defined -> fresh visit<>

int main() {
  bar::visit<bar::mfn, bar::holder<bar::box<>>>({});  // control: expect pass
  bar::visit<ifn,      bar::holder<bar::box<>>>({});  // suspect
}
