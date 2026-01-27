using Core;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Scripting
{
    public class CoinComponent : Component
    {
        private PlayerComponent _player;

        public CoinComponent(GameObject gameObject, string name) : base(gameObject, name)
        {
            // do not use ctor as a start, use only for initialization
        }

        void Start()
        {
            foreach (var keyValuePair in NativeBridge.CreatedGameObjects)
            {
                _player = keyValuePair.Value.Components.FirstOrDefault(c => c is PlayerComponent) as PlayerComponent;
                if (_player != null)
                {
                    _player.TotalCoins++;
                    break;
                }
            }
        }

        void Update(float deltaTime)
        {
            if ((_player.transform.position - transform.position).sqrMagnitude < 1f)
            {
                transform.position = new Vector3(0, -1000000, 0);
                _player.CollectCoin();
            }
        }
    }
}
