# MAD Engine
(aka the MAD DAMAGE Engine)

MAD Engine is an educational game engine developed to explore modern techniques and technology used within specific domains, most notably Computer Graphics and Game Networking.
Within the Computer Graphics domain, MAD Engine will explore specific sub-domains such as deferred shading, dynamic lights, dynamic shadows, and particle systems.
Within the Game Networking domain, MAD Engine does not try to reinvent the wheel with low-level networking implementations, but rather focuses on a higher level reliable UDP networking system that supports features such as lag compensation, client-side prediction, server-authoritative control, remote procedure calls, and related topics.

We hope to use as few 3rd party dependencies as possible. Current dependencies include:

+ DirectX 11
+ [DirectXTK](https://github.com/Microsoft/DirectXTK) (SimpleMath, [WIC/DDS]TextureLoader)
+ [Premake](https://github.com/premake/premake-core) (Windows binary included in the repository)
+ [assimp](https://github.com/assimp/assimp) (model importing)
+ [rapidjson](https://github.com/miloyip/rapidjson) (world loading)
+ [libyojimbo](https://github.com/networkprotocol/libyojimbo) (low-level game networking solution)

We have a [Trello](https://trello.com/b/pOoAXZ8c) if you'd like to know what we're currently working on (it's totally out of date at the moment); things aren't yet sorted by priority, but we're getting to that soon. Upcoming work and priorities are of course subject to change as we come across things we're more/less interested in getting done.
