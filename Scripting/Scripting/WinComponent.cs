
using Core;

namespace Scripting
{
    public class WinComponent : Component
    {
        private Vector3 _initialPosition;
        
        public WinComponent(GameObject gameObject, string name) : base(gameObject, name)
        {
        }

        public void Show()
        {
            transform.position = _initialPosition;
        }

        void Start()
        {
            _initialPosition = transform.position;
            transform.position = new Vector3(0, 100000, 0);
        }

        void Update(float deltaTime)
        {
        }
    }
}