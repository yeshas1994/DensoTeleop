#include"stdafx.h"
#include <stdio.h>

#include "dhdc.h"
#include "drdc.h"

//header files for communication
//#include<Windows.h>
#include<WinSock2.h>
#include <iostream>
#include <WS2tcpip.h>


//header files for string computations
#include<conio.h>
#include<cstdio>
#include<cstdlib>
#include<cstring>
#include<iostream>
#include<sstream>

//to include the winsock library
#pragma comment(lib, "ws2_32.lib")

#define k 10000
WSADATA data;
//further intialising
WORD version = MAKEWORD(2, 2);

int computeForce(double px, double py, double pz, double *fx, double *fy, double *fz) {
	
	*fx  = k * px;
	*fy = k * py;
	*fz = k * pz;

	//if (pz > 0.05) return 1;
	//else
	return 0;
	
}

//class for communicating the joint angles locally
class comm_TCP {
	private:
		//private datatype to send data
		std::string sendData;
		SOCKET sserver;
		SOCKET client;
		sockaddr_in server;
		double dP(double val) {
			return ((double)(((int)((val*180/3.14159) * 1000)))) / (double)1000;
		}
		double prec(double val) {
			return ((double)(((int)((val * 100))))) / (double)100;
		}

	public:
	
		//converts the double value to three decimal places for transfer  
		
		//initialises data to string
		void initData(double angles[DHD_MAX_DOF],int status, double px, double py, double pz) {
			sendData="";
			
			for (int i = 0; i < DHD_MAX_DOF-1; i++) {       
				std::stringstream ss;
				if (i > 2) {
					ss << dP(angles[i]);
				}
				else if(i==0) {
					ss << prec((px*1000));
				}
				else if (i == 1) {
					ss << prec((py*1000));
				}
				else if (i == 2) {
					ss << prec((pz*1000));
				}
				sendData += ss.str() + ",";
				ss.clear();
			}
			std::stringstream ss;
			ss << status;
			sendData += ss.str() + ",";
			
			printf("Output String: %s\n", sendData.c_str());
			
		}
		void initData(double angles[DHD_MAX_DOF], int status) {
			sendData = "";

			for (int i = 0; i < DHD_MAX_DOF - 1; i++) {
				std::stringstream ss;
				ss << dP(angles[i]);
				sendData += ss.str() + ",";
				ss.clear();
			}
			std::stringstream ss;
			ss << status;
			sendData += ss.str() + ",";

			printf("Output String: %s\n", sendData.c_str());

		}
		comm_TCP() {
			//some varaibles to intialise winsock stream (Windows Socket Implementation)
		/*	WSADATA data;
			//further intialising
			WORD version = MAKEWORD(2, 2);
			*/
			// Start WinSock
			int wsOk = WSAStartup(version, &data);
			if (wsOk != 0)
			{
				// Not ok! Get out quickly
				std::cerr<< "Can't start Winsock! " << wsOk;
			}
		}

		boolean SocketServer() {
			//TCP is a protocol that facilitates data transfer
			//need to initialise socket to the required address with port number to enable connection


			// Create a hint structure for the server
		

			//filling the socket adrres structure with zeros to prevent garbage values
			//memset(&server, 0, sizeof(server));
			SOCKET sserver = socket(AF_INET, SOCK_STREAM,IPPROTO_TCP);
			server.sin_family = AF_INET; // AF_INET = IPv4 addresses
			server.sin_port = htons(11080); // Little to big endian conversion which is the TCP/IP network byte order
			server.sin_addr.S_un.S_addr = htonl(INADDR_ANY);
											//change the address accordingly for communication
			//inet_pton(AF_INET, INADDR_ANY, &server.sin_addr); // Convert from string to byte array : aloocating an adress
															  // Socket creation, note that the socket type is datagram
			
			if (sserver == INVALID_SOCKET) {
				std::cerr<<"Could not send: Socket not intialised\n";
				closesocket(sserver);

				// Close down Winsock
				WSACleanup();
				return false;
			}
			
			//bind links the socket we just created with the sockaddr_in 
			//structure. Basically it connects the socket with 
			//the local address and a specified port.
			//If it returns non-zero quit, as this indicates error
			if (bind(sserver, (struct sockaddr*)&server, sizeof(server))!=0)
			{
				closesocket(sserver);
				std::cerr << "Bound error\n";
				// Close down Winsock
				WSACleanup();
				return 0;
			}
			else printf("Bound   ");
			//listen instructs the socket to listen for incoming 
			//connections from clients. The second arg is the backlog
			if (listen(sserver, SOMAXCONN) != 0)
			{
				closesocket(sserver);
				std::cerr << "Error in listening\n";
				// Close down Winsock
				WSACleanup();
				return 0;
			}
			else printf("Listening...\n");

			sockaddr_in clientadd;
			int c = sizeof(clientadd);
			client = accept(sserver, ( struct sockaddr*)&clientadd, &c);
			if (client == INVALID_SOCKET) {
				printf("Error in client recieve: %d\n",WSAGetLastError());
				return 0;
			}
			else printf("Accepted\n");
			printf("Received Connection\n");
			fflush(stdin);
		}

		boolean SocketClient() {
			
			SOCKET ConnectSocket = INVALID_SOCKET;
			sockaddr_in	hints;
			
			int iResult;
		
			//ZeroMemory(hints, sizeof(hints));
			hints.sin_family = AF_INET;
			hints.sin_port = htons(11080);
		
			inet_pton(AF_INET, "127.0.0.1", &hints.sin_addr);
			// Resolve the server address and port
			/*iResult = getaddrinfo(arg, DEFAULT_PORT, &amp; hints, &amp; result);
			if (iResult != 0) {
				printf("getaddrinfo failed with error: %d\n", iResult);
				WSACleanup();
				return 1;
			}
			*/
			// Attempt to connect to an address until one succeeds
			int clen = sizeof(hints);
				// Create a SOCKET for connecting to server
			client = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
				if (client == INVALID_SOCKET) {
					printf("socket failed with error: %ld\n", WSAGetLastError());
					WSACleanup();
					return false;
				}
				printf("Client socket established\n");
				// Connect to server.
				iResult = connect(client, (struct sockaddr*)&hints, clen);
				if (iResult == SOCKET_ERROR) {
					closesocket(ConnectSocket);
					ConnectSocket = INVALID_SOCKET;
					return false;
				}
			
				printf("Connected to address\n");
			

			if (client == INVALID_SOCKET) {
				printf("Unable to connect to server!\n");
				WSACleanup();
				return false;
			}

			// Send an initial buffer
			return true;
		}

		boolean Data(){

		//	listening to a connection
		
		
		if (send(client, sendData.c_str(), 64, 0) == SOCKET_ERROR) {
			printf("Data Not Sent");
			return false;
		}
		fflush(stdin);
		printf("Sent  \n");

		//closesocket(client);
		return true;
		// Close the socket
	
	}
		~comm_TCP() {
			closesocket(sserver);
			// Close down Winsock
			WSACleanup();
			
		}
};
double toDeg(double rad) {
	return rad * (180 / 3.14);
}

int main(int argc, char **argv) {

	int    done = 0;
	double px, py, pz;
	double fx, fy, fz;
	double oa, ob, og;
	double f[DHD_MAX_DOF];
	double p[DHD_MAX_DOF];
	double angles[DHD_MAX_DOF];
	double forces[3];
	double AngX, AngZ, AngDev, AngGrip;

	if (dhdGetDeviceCount() <= 0) {
		printf("error: no device found (%s)\n", dhdErrorGetLastStr());
		return 0;
	}
	if (dhdOpen() < 0) {
		printf("error: cannot open device\n");
		return 0;
	}

	if (dhdEmulateButton(1) < 0) {
		printf("error: could not emulate button");
		return 0;
	}
	
	dhdEnableExpertMode();

	comm_TCP TCP_Object;
	if (TCP_Object.SocketClient()) {
		printf("Socket Not Bound\n");
	}
	 
	int ctr = 0;
	while (ctr<100 && done!=1) {
			
		dhdGetPositionAndOrientationDeg(&px, &py, &pz, &oa, &ob, &og);
		done = computeForce(px, py, pz, &fx, &fy, &fz);
		printf("Position: %.2lf %.2lf %.2lf %.2lf %.2lf %.2lf \n", px, py, pz, oa, ob, og);
		dhdGetGripperAngleDeg(&AngGrip);
		//if (AngGrip > 25) done = 1;
		dhdGetJointAngles(angles);
		for (int i = 0; i < DHD_MAX_DOF - 1; i++)
			printf(" Joint Angles:%d %.2lf\n ",i+1, toDeg(angles[i]));

		//communications
	
		TCP_Object.initData(angles,done,px,py,pz);
		TCP_Object.Data();
			
		//dhdDisableExpertMode();
		/*dhdGetBaseAngleXDeg(&AngX);
		dhdGetBaseAngleZDeg(&AngZ);
		dhdGetDeviceAngleDeg(&AngDev);
		
		printf("Position: %lf %lf %lf %lf \n", AngX, AngZ, AngDev, AngGrip);*/
		//apply forces
		dhdSetForce(fx, fy, fz);
		
		//done = dhdGetButton(0);
		dhdSleep(1);
		ctr++;
	}
	printf("exiting application\n");
	dhdDisableExpertMode();
	dhdStop();
	dhdClose();


	
	return 0;
}