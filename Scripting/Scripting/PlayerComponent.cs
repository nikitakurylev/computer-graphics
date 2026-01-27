using Core;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Scripting
{
    public class PlayerComponent : Component
    {
        private GameObject player;

        public PlayerComponent(GameObject gameObject, string name) : base(gameObject, name)
        {
            // do not use ctor as a start, use only for initialization
        }

        void Start()
        {
        }

        void Update(float deltaTime)
        {
        }
    }
}
