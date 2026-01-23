using Core;

namespace Scripting
{
    public class MovementComponent : Component
    {
/*        [MethodImpl(MethodImplOptions.InternalCall)]
        extern static string gimme();*/

        public void Main2()
        {
            //System.Console.WriteLine(gimme());
            System.Console.WriteLine("1");
/*
            if (gimme().Equals("All your monos are belong to us!"))
                return 0;*/

        }

        void Update()
        {
            System.Console.WriteLine("Update"); 
        }

    }
}
