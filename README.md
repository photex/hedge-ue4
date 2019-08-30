# hedge-ue4
Putting some of my ideas down into a UE4 context.

Hedge is meant to be a set of core element structures that are based on the half-edge mesh. h-edge... get it? :D
The goal or the idea behind Hedge was to work on a sort of modeling tool foundation that puts together some kind
of fluent api to build and modify meshes. 

With an intuitive or at least interesting foundation that can be hoisted up into Blueprints, you have some legwork for 
building even more interesting or novel tools in the editor. I'm sort of looking at this from the point of view of an 
artist working with a system that is of a fashion similar to SOPs in Houdini.

Why bother with this? Because it's extremely interesting to me and I think having some interesting tools to actually
make shapes in the editor/engine directly can open the doors for some creative minds to flex in new ways. :+1:
In the Rust version it's meant to serve as a vehicle for people who want to explore some procedural modeling with the
Amethyst engine. In the vanilla c++ version I'm mostly just implementing the same api and seeing how my thoughts
change when the rules of the language change. For this repo it's useful to know if any of my ideas survive being put
to work in a big interactive system.


*But I don't get paid to do this work, and I never have any kind of dedicated sprint or push towards a specific goal
so this repo is probably not always in the best state. And if I'm being honest, likely will never be finished or
come to any fruition.*


UE4 currently has a set of modules that provide a ton of functionality and types. 
The UE4 mesh description and editable mesh modules have their own structures and approaches for representing meshes.
They include serialization and all the stuff you'd expect from a proper tool, and honestly a ton of what has started
out here will or can already be replaced by the more mature work in those modules. I don't think the higher-level
ideas I have for Hedge are incompatible with what already exists in the engine, I just like to get my hands dirty and 
dig around for a while so I can better understand things. Also, I haven't actually poked around some of the other
development branches and my understanding so far of what's present in the engine is based on the current release (4.22.x).
