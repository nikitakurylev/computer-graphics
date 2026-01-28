![screenshot](screenshot.png)

# Motor Game Engine

A game engine created as a part of computer graphics course in ITMO University.

## Features

* C# Scripting
* .GLB as scene format
* PBR support
* Audio and Physics systems
* Behaviour tree with Utility AI

## Getting Started

### Dependencies

* Either [vcpkg](https://github.com/microsoft/vcpkg) or a fresh version of [assimp](https://github.com/assimp/assimp) installed
* [Mono](https://www.mono-project.com/download/stable/)
* Visual Studio with "Game Development with C++" installed

### Executing program

1) Open and build `/Scripting/Scripting/Scripting.sln` solution
2) Open and run `computer-graphics.sln` solution

### Scripting
Creating Custom Components

  To create your own component, make a new class that inherits from Core.Component. Implement the constructor and initialize your script values there.
    Example:
    
    namespace Scripting
    {
        public class WinComponent : Component
        {
            public WinComponent(GameObject gameObject, string name) : base(gameObject, name)
            {
                // Initialize your component here
            }
        }
    }

  Implement the standard methods void Update(float deltaTime) and void Start() as needed. Their visibility (public/private) is not important.
    Example:

    public class WinComponent : Component
    {
        private Vector3 _initialPosition;
        
        public WinComponent(GameObject gameObject, string name) : base(gameObject, name)
        {
            // Initialization
        }

        void Start()
        {
            Console.WriteLine("Start!");
            _initialPosition = transform.position;
        }

        void Update(float deltaTime)
        { 
            transform.position += Vector3.forward * deltaTime; 
        }
    }

  Build the assembly
    Build your solution to generate the .dll file:
        In Visual Studio: Build → Build Solution
        The compiled assembly will be available in the bin/directory of your project

## Authors

[Dmitry Kulikov](https://github.com/W00fdev) - C# scripting and general engine architechture

[Platon Iofinov](https://github.com/elToton) - Rendering and material import

[Maxim Iakovlev](https://github.com/Utka-EmpeROR) - Behaviour Tree and Utility AI

[Nikita Kurylev](https://github.com/nikitakurylev) - .GLB import, audio and physics system
