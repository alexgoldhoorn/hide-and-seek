#include "Component.h"

using namespace std;


Component::Component(void)
{
}

Component::~Component(void)
{
}

Component* Component::getParentComponent()
{
    return parent;
}

void Component::add(Component *childComponent)
{
    subComponents.push_back(childComponent);
    childComponent->parent = this;
}
string Component::getName()
{
    return name;
}

Component* Component::getComponent(string name)
{
    for(vector<Component *>::iterator iter = subComponents.begin(); iter != subComponents.end() ; iter ++)
    {
        if((*iter)->getName().compare(name) == 0)
        {
            return *iter;
        }
    }
    return NULL;
}

