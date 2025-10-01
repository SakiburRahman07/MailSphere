
package project;

channel P2P extends ned.DatarateChannel
{
    datarate = 100Mbps;
    delay = 0.2ms;
}

simple Router
{
    parameters:
        string routes = default("");   // e.g. "1:0,2:1,3:2"
        @display("i=device/router");
    gates:
        inout pppg[3];                 // <-- fixed-size vector gate (0..2)
}

//
// TODO documentation
//
simple MTA_Server_RS
{
    parameters:
        int address;
    @display("i=device/server2,,0");
    gates:
        inout ppp[2];
}

//
// TODO documentation
//
simple MTA_Client_SS
{
    parameters:
        int address;
    @display("i=device/server2,,0");
    gates:
        inout ppp[2];
}

//
// TODO documentation
//
simple MAA_Server
{
    parameters:
        int address;
    @display("i=abstract/server,,0");
    gates:
        inout ppp[2];
}

//
// TODO documentation
//
simple MAA_Client
{
    parameters:
        int address;
    @display("i=abstract/server,,0");
    gates:
        inout ppp[2];
}

//
// TODO documentation
//
simple spool
{
    parameters:
        int address;
    @display("i=block/queue,,0");
    gates:
        inout ppp[2];
}

//
// TODO documentation
//
simple mailbox
{
    parameters:
        int address;
    @display("i=old/mailbox,,0");
    gates:
        inout ppp[2];
}



//
// TODO documentation
//
simple Sender
{
    parameters:
        int address;
        string message = default("Hello from Sender");
    @display("i=device/laptop,,0");
    gates:
        inout ppp;
}

//
// TODO documentation
//
simple Receiver
{
    parameters:
        int address;
    @display("i=device/pc,,0");
    gates:
        inout ppp;
}

simple DNS
{
    parameters:
        int address;
        int answerAddr = default(3);
        @display("i=device/server");
    gates:
        inout ppp;
}

simple HTTP
{
    parameters:
        int address;
        double serviceTime @unit(s) = default(5ms);
        int pageSizeBytes = default(20000);
        @display("i=device/server");
    gates:
        inout ppp;
}

//
// TODO documentation
//
simple MTA_Client_S
{
    parameters:
        int address;
    @display("i=old/server1,,0");
    gates:
        inout ppp[2];
}

//
// TODO documentation
//
simple MTA_Server_S
{
    parameters:
        int address;
    @display("i=old/server1,,0");
    gates:
        inout ppp[2];
}

network SimpleNet
{
    @display("bgb=1162,561");
    submodules:
        spool: spool {
            @display("p=401,320");
        }
        mta_Client_SS: MTA_Client_SS {
            @display("p=401,458");
        }
        router: Router {
            @display("p=604,458");
        }
        dns: DNS {
            @display("p=604,330");
        }
        mta_Server_RS: MTA_Server_RS {
            @display("p=789,458");
        }
        mailbox: mailbox {
            @display("p=789,330");
        }
        maa_Server: MAA_Server {
            @display("p=789,199");
        }
        maa_Client: MAA_Client {
            @display("p=789,62");
        }
        receiver: Receiver {
            @display("p=1082,62");
        }
        sender: Sender {
            @display("p=81,60");
        }
        mta_Client_S: MTA_Client_S {
            @display("p=401,61");
        }
        mta_Server_S: MTA_Server_S {
            @display("p=401,199");
        }
        router1: Router {
            @display("p=240,62");
        }
        router2: Router {
            @display("p=951,62");
        }
        dns1: DNS {
            @display("p=240,210");
        }
        dns2: DNS {
            @display("p=951,210");
        }
    connections:
        mta_Client_S.ppp[1] <--> P2P <--> mta_Server_S.ppp[0];
        mta_Server_S.ppp[1] <--> P2P <--> spool.ppp[0];
        spool.ppp[1] <--> P2P <--> mta_Client_SS.ppp[0];
        mta_Client_SS.ppp[1] <--> P2P <--> router.pppg[0];
        router.pppg[1] <--> P2P <--> dns.ppp;
        router.pppg[2] <--> P2P <--> mta_Server_RS.ppp[0];
        mta_Server_RS.ppp[1] <--> P2P <--> mailbox.ppp[0];
        mailbox.ppp[1] <--> P2P <--> maa_Server.ppp[0];
        maa_Server.ppp[1] <--> P2P <--> maa_Client.ppp[0];
        maa_Client.ppp[1] <--> P2P <--> router2.pppg[2];
        sender.ppp <--> P2P <--> router1.pppg[0];
        router1.pppg[1] <--> P2P <--> dns1.ppp;
        mta_Client_S.ppp[0] <--> P2P <--> router1.pppg[2];
        receiver.ppp <--> P2P <--> router2.pppg[0];
        router2.pppg[1] <--> P2P <--> dns2.ppp;
}

