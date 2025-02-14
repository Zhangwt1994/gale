#include <json_spirit_reader.h>
#include <stdexcept>

class Compare_Name
{
public:
  std::string name;
  Compare_Name(const std::string &n): name(n) {}
  bool operator()(json_spirit::Pair p)
  {
    return p.name_==name;
  }
};

void add_components(json_spirit::Object &o,
                    const json_spirit::Object &components)
{
  /* Only add a component if it is not already there */
  json_spirit::Object temp;
  for(json_spirit::Object::const_iterator i=components.begin();
      i!=components.end(); ++i)
    {
      if(std::find_if(o.begin(),o.end(),Compare_Name(i->name_))==o.end())
        {
          temp.push_back(*i);
        }
    }
  temp.insert(temp.end(),o.begin(),o.end());
  temp.swap(o);
}
