
#include "common_objects.h"
#include "UParser.h"
#include <algorithm>

TypedList::TypeGroup & TypedList::operator[](const std::string & group_name)
{
    auto it = std::find_if(data.begin(), data.end(), [=](const TypeRecord & rec){
        return rec.first == group_name;
    });
    
    if(it == data.end())
    {
        //throw parser_exception("Unable to locate given type group");
        TypeRecord new_rec;
        new_rec.first = group_name;
        data.push_back(new_rec);
        return data[data.size()-1].second;
    }
    else
        return it->second;
}
