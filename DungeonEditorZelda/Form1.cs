using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;
using System.IO;

namespace DungeonEditorZelda
{
    public partial class Form1 : Form
    {
        public Form1()
        {
            InitializeComponent();
        }

        private void Form1_Load(object sender, EventArgs e)
        {
            FileStream fs = new FileStream("zelda.sfc", FileMode.Open, FileAccess.Read);
            ROM_DATA = new byte[fs.Length];
            fs.Read(ROM_DATA, 0, (int)fs.Length);
            fs.Close();

            Decompress(0);
        }
        byte[] ROM_DATA;
        List<byte> dataBuffer = new List<byte>();
        public void Decompress(int apos)
        {
            int pos = 0x08B800;
            bool done = false;
            
            while (done == false)
            {
                bool expand = false;
                byte b = ROM_DATA[pos];
                
                Console.WriteLine(b + " : " +((b >> 5) & 0x07));
                if (b == 0xFF)
                {
                    done = true;
                }

                if (((b >> 5) & 0x07) == 7) //expanded command
                {
                    expand = true;
                }
                byte cmd = 0;
                short length = 0;
                if (expand)
                {
                    cmd = (byte)((b >> 2) & 0x07);
                    length = BitConverter.ToInt16(new byte[] { (ROM_DATA[pos + 1]), (byte)(b & 0x03) }, 0);
                    pos += 2;
                }
                else
                {
                    cmd = (byte)((b >> 5) & 0x07);
                    length = (byte)(b & 0x1F);
                    pos += 1;
                }
                length += 1;
                if (cmd == 0)//000 Direct Copy Followed by (L+1) bytes of data
                {
                    for(int i = 0;i<length;i++)
                    {
                        dataBuffer.Add(ROM_DATA[pos]);
                        pos++;
                    }
                }
                
       

                if (cmd == 1)//001 "Byte Fill" Followed by one byte to be repeated (L + 1) times
                {
                    byte copiedByte = ROM_DATA[pos];
                    pos++;
                    for (int i = 0; i < length; i++)
                    {
                        dataBuffer.Add(copiedByte);
                    }
                    //pos++;
                }


                if (cmd == 2)//010    "Word Fill" Followed by two bytes. Output first byte, then second, then first,then second, etc. until (L+1) bytes has been outputted
                {
                    byte copiedByte = ROM_DATA[pos];
                    byte copiedByte2 = ROM_DATA[pos+1];
                    pos += 2;
                    int j = 0;
                    for (int i = 0; i < length; i++)
                    {
                        if (j == 0)
                        {
                            dataBuffer.Add(copiedByte);
                            j = 1;
                        }
                        else
                        {
                            dataBuffer.Add(copiedByte2);
                            j = 0;
                        }
                    }
                }
                

                if (cmd == 3)//"Increasing Fill" Followed by one byte to be repeated (L + 1) times, but the byte is increased by 1 after each write
                {
                    byte copiedByte = ROM_DATA[pos];
                    pos += 1;
                    for (int i = 0; i < length ; i++)
                    {
                        dataBuffer.Add((byte)(copiedByte +i));
                    }
                }

                if (cmd == 4)//"Repeat" Followed by two bytes (ABCD byte order) containing address (in the output buffer) to copy (L + 1) bytes from
                {
                    byte copiedByte = ROM_DATA[pos];
                    byte copiedByte2 = ROM_DATA[pos+1];
                    int pos2 = BitConverter.ToInt16(new byte[] { copiedByte, copiedByte2 }, 0);
                    pos += 2;
                    for (int i = 0; i < length; i++)
                    {
                        dataBuffer.Add(dataBuffer[pos2]);
                        pos2++;
                    }
                }





            }
            FileStream fs = new FileStream("gfx.bin", FileMode.OpenOrCreate, FileAccess.Write);
            fs.Write(dataBuffer.ToArray(), 0, dataBuffer.Count);
            fs.Close();

        }
    }
}
