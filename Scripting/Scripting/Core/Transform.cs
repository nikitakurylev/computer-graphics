using System;
using System.Runtime.CompilerServices;

namespace Core
{
    public sealed class Transform
    {
        // fields reading from engine

        public Vector3 position;
        //public Quaternion scale;
        public Vector3 scale;
        public Int32 uid => _uid;

        private Int32 _uid;

        private Transform(Int32 uid, Vector3 position, Vector3 scale)
        {
            _uid = uid;

            UpdateTransform(position, scale);
        }

        private void UpdateTransform(Vector3 position, Vector3 scale)
        {
            this.position = position;
            this.scale = scale;
        }
    }
}
