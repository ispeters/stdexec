import std;
import repro;

int main()
{
  ns::thing t;
  int       result = t.run([](int x) { return x * 2; });
  std::printf("result = %d\n", result);
  return result == 84 ? 0 : 1;
}
