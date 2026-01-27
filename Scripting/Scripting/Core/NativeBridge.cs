using System;
using System.Collections.Generic;
using System.Reflection;
using System.Runtime.CompilerServices;

namespace Core
{
    static class NativeBridge
    {
        // int32 uid is shared between transform and gameObject
        private static readonly Dictionary<string, Type> _componentTypes = new Dictionary<string, Type>();
        private static readonly Dictionary<Int32, GameObject> _createdGameObjects = new Dictionary<Int32, GameObject>();
        private static readonly Dictionary<Int32, Transform> _createdTransforms = new Dictionary<Int32, Transform>();

        static NativeBridge()
        {
            Assembly assembly = Assembly.GetExecutingAssembly();
            Type componentBaseType = typeof(Core.Component);
            Type gameObjectBaseType = typeof(Core.GameObject);

            foreach (Type type in assembly.GetTypes())
            {
                if (type.IsClass && !type.IsAbstract && componentBaseType.IsAssignableFrom(type))
                {
                    _componentTypes[type.Name] = type;

                    Console.WriteLine($"[BRIDGE] Registered component type: '{type.Name}'");
                }
            }
        }

        [MethodImpl(MethodImplOptions.NoInlining)]
        private static Transform CreateTransform(Int32 uid, Vector3 position, Vector3 scale)
        {
            if (_createdTransforms.ContainsKey(uid))
            {
                throw new ArgumentException($"[BRIDGE] transform with uid '{uid}' already created");
            }

            var transform = (Transform)Activator.CreateInstance(
                typeof(Transform),
                BindingFlags.NonPublic | BindingFlags.Instance,
                null,
                new object[] { uid, position, scale},
                null
            );

            _createdTransforms.Add(uid, transform);

            Console.WriteLine($"[BRIDGE] Created transform uid: '{uid}', (at='{position}'), (scale='{scale}')");
            return transform;
        }

        [MethodImpl(MethodImplOptions.NoInlining)]
        private static GameObject CreateGameObject(Int32 uid, string name)
        {
            if (_createdGameObjects.ContainsKey(uid))
            {
                throw new ArgumentException($"[BRIDGE] game object with (uid='{uid}'), (name='{name}') already created");
            }

            if (!_createdTransforms.TryGetValue(uid, out Transform transform))
            {
                throw new ArgumentException($"[BRIDGE] trying to create game object with name '{name}' without created transform.\nGiven uid: '{uid}'");
            }

            var gameObject = (GameObject)Activator.CreateInstance(
                typeof(GameObject),
                BindingFlags.NonPublic | BindingFlags.Instance,
                null,
                new object[] { uid, name, transform},
                null
            );

            _createdGameObjects.Add(uid, gameObject);

            return gameObject;
        }

        [MethodImpl(MethodImplOptions.NoInlining)]
        private static Component CreateComponent(Int32 objectUid, string componentName)
        {
            if (!_componentTypes.TryGetValue(componentName, out var type))
            {
                throw new ArgumentException($"[BRIDGE] Component type '{componentName}' not found");
            }

            if (!_createdGameObjects.TryGetValue(objectUid, out GameObject gameObject))
            {
                throw new ArgumentException($"[BRIDGE] GameObject uid '{objectUid}' not found");
            }

            var component = (Component)Activator.CreateInstance(
                type,
                BindingFlags.Public | BindingFlags.Instance,
                null,
                new object[] { gameObject, componentName },
                null
            );

            Console.WriteLine($"[BRIDGE] Created component type: '{type.Name}'");

            return component;
        }
    }
}
