/* $Id: PalmServer.java,v 1.1 2002/09/15 07:46:30 pgr Exp $ */

import java.net.*;
import java.io.*;

public class PalmServer
{
   private Socket socket;
   boolean alive;
   Receiver rcvr;
   Sender sndr;
   /*--------------------------------------------------------------PalmServer-+
   *
   +-------------------------------------------------------------------------*/
   public PalmServer(ServerSocket serverSocket) throws IOException
   {
      System.out.println(
         "PalmServer waiting for on port " + serverSocket.getLocalPort()
      );
      socket = serverSocket.accept();
      System.out.println(
         "Connected to " + socket.getInetAddress() + ':' + socket.getPort()
      );
      rcvr = new Receiver();
      sndr = new Sender();
   }

   /*-------------------------------------------------------------------start-+
   *
   +-------------------------------------------------------------------------*/
   public void start() throws IOException
   {
      alive = true;
      rcvr.start();
      sndr.start();
      while (rcvr.isAlive()) {
         try { Thread.currentThread().sleep(500); }catch (InterruptedException e) {}
      }
      alive = false;
      if (sndr.isAlive()) sndr.interrupt();
      try { socket.close(); }catch (IOException e) {}
      System.out.println("Connection closed.");
   }

   /*----------------------------------------------------------class Receiver-+
   *
   +-------------------------------------------------------------------------*/
   class Receiver extends Thread
   {
      private InputStream input;

      Receiver() throws IOException {
         input = socket.getInputStream();
      }

      public void run()
      {
         // print received data
         try {
            while (alive) {
               int c = input.read();
               if (c == -1) break;
               System.out.print((char)c);
            }
         }catch (IOException e) {
            System.out.println(e);
         }
         System.out.println("Receiver ended.");
      }
   }

   /*------------------------------------------------------------class Sender-+
   *
   +-------------------------------------------------------------------------*/
   class Sender extends Thread
   {
      BufferedReader input;
      PrintWriter output;

      Sender() throws IOException {
         input = new BufferedReader(new InputStreamReader(System.in));
         output = new PrintWriter(socket.getOutputStream(), true);
      }

      public void run()
      {
         // get user input and transmit it
         try {
            while (alive) {
               if (0 == System.in.available()) {
                  try { sleep(300); }catch (InterruptedException e) {}
                  continue;
               }
               String lineToSend = input.readLine();
               // stop if input line is "."
               if (lineToSend.equals(".")) break;
               output.println(lineToSend);
            }
         }catch (IOException e) {
             System.out.println(e);
         }
         System.out.println("Sender ended...");
      }
   }

   /*--------------------------------------------------------------------main-+
   *
   +-------------------------------------------------------------------------*/
   public static void main(String args[])
   {
      int port;

      try {
         port = Integer.parseInt(args[0]);
      }catch (Exception e) {
         port = 6416;
      }

      try {
         ServerSocket serverSocket = new ServerSocket(port);
         for (;;) {
            new PalmServer(serverSocket).start();
         }
      }catch (IOException e) {
         System.out.println(e);
      }
   }
}

/*===========================================================================*/



