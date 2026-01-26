using Core;

namespace Scripting
{
    public class MovementComponent : Component
    {
        public MovementComponent(GameObject gameObject, string name) : base(gameObject, name)
        {
            // do not use ctor as a start, use only for initialization
        }


        /*[MethodImpl(MethodImplOptions.InternalCall)]
       extern static string gimme();*/

        public void Main2()
        {
            //System.Console.WriteLine(gimme());
            System.Console.WriteLine("1");
/*
            if (gimme().Equals("All your monos are belong to us!"))
                return 0;*/

        }

        void Update(Vector2 vector2)
        {
            System.Console.WriteLine(vector2); 
        }

    }
}
