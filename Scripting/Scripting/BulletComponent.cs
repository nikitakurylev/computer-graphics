using Core;

namespace Scripting
{
    public class BulletComponent : Component
    {
        public BulletComponent(GameObject gameObject, string name) : base(gameObject, name)
        {
            // do not use ctor as a start, use only for initialization
        }

        void Start()
        {
            System.Console.WriteLine("Start");
        }

        void Update(float deltaTime)
        {
            transform.position += Vector3.forward * deltaTime;
        }
    }
}
