using System;
using System.Runtime.CompilerServices;

namespace Core
{
    public sealed class GameObject
    {
        public Transform transform => _transform;
        public string name;
        public Int32 uid => _uid;

        // no components here, only engine-side component controls

        private readonly Int32 _uid;
        private readonly string _name;
        private readonly Transform _transform;

        [MethodImpl(MethodImplOptions.NoInlining)]
        private GameObject(Int32 uid, string name, Transform transform)
        {
            _uid = uid;
            _name = name;
            _transform = transform;

            Console.WriteLine($"Transform created {uid} {name}");
        }
    }
}
