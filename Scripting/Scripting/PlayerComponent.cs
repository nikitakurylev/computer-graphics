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
        public int TotalCoins { get; set; }
        
        private GameObject player;

        public PlayerComponent(GameObject gameObject, string name) : base(gameObject, name)
        {
            // do not use ctor as a start, use only for initialization
        }

        public void CollectCoin()
        {
            TotalCoins--;
            if (TotalCoins == 0)
                foreach (var keyValuePair in NativeBridge.CreatedGameObjects)
                {
                    if (keyValuePair.Value.Components.FirstOrDefault(c => c is WinComponent) is WinComponent win)
                    {
                        win.Show();
                        break;
                    }
                }
        }

        void Start()
        {
        }

        void Update(float deltaTime)
        {
        }
    }
}
