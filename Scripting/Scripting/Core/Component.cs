using System;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;

namespace Core
{
    public abstract class Component : IDisposable
    {
        [DllImport("__Internal")]
        private static extern void ReleaseNativeComponent(IntPtr ptr);

        public Transform transform => gameObject.transform;
        public GameObject gameObject { get; private set; }
        public string name { get; private set; }

        protected IntPtr _nativePtr = IntPtr.Zero;
        private bool _disposed;

        [MethodImpl(MethodImplOptions.NoInlining)]
        protected Component(GameObject gameObject, string name)
        {
            this.gameObject = gameObject;
            this.name = name;

            Console.WriteLine($"Transform created {gameObject.uid} {name}");
        }

        [MethodImpl(MethodImplOptions.NoInlining)]
        protected void SetNativePtr(IntPtr nativePtr)
        {
            _nativePtr = nativePtr;
        }

        ~Component() => Dispose();

        public void Dispose()
        {
            if (!_disposed)
            {
                if (_nativePtr != IntPtr.Zero)
                {
                    ReleaseNativeComponent(_nativePtr);

                    _nativePtr = IntPtr.Zero;
                }

                _disposed = true;

                GC.SuppressFinalize(this);
            }
        }
    }
}
