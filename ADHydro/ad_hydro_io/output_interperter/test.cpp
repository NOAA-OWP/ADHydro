#include "interperter.h"
#include "adhydro_boost_types.h"
#include <unordered_map>
int main(int argc, char** argv)
{
  //IOSpecReader reader;
  //reader.readSpec();
  rtree r1, r2;
  std::unordered_map<int, bool> test;
  Interperter interperter("output_spec", r1, r2, test, test);
  interperter.interpert();
}
