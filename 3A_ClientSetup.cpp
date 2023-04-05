#include <iostream>
#include <mysql.h>
#include <string>
#include <fstream>
using namespace std;

//Config
#define ServerIP "192.168.18.5"
#define DbName "magang-database"
#define DbUsername "client"
#define DbPassword ""
#define DbPort 3306

MYSQL* conn; MYSQL_ROW row; MYSQL_RES* res;
string query; int clientID, qState;

void connectToDatabase() {
	conn = mysql_init(0);
	conn = mysql_real_connect(conn, ServerIP, DbUsername, DbPassword, DbName, DbPort, NULL, 0);
	while (!conn) {
		cout << "Error connecting to Database, Reconnecting...\n";
		conn = mysql_init(0);
		conn = mysql_real_connect(conn, ServerIP, DbUsername, DbPassword, DbName, DbPort, NULL, 0);
		Sleep(3000);
	}
	cout << "Connected to Database!\n";
}
string sqlQuery(string query) {
	string disconnect = "", qOutput = "";
	const char* q = query.c_str();
	qState = mysql_query(conn, q);

	if (!qState) {
		res = mysql_store_result(conn);
		if (res != NULL) {
			qOutput = "";
			while (row = mysql_fetch_row(res)) {
				int j = res->field_count;
				for (int i = 0; i < j; i++) {
					string x = row[i];
					qOutput += x + '/';
				}
				qOutput += '\n';
			}
		}
	}
	else {
		string sqlErr = mysql_error(conn);
		cout << "Database error: " << sqlErr << endl;
		cout << "Trying to Reconnecting to the Database...\n";

		do {
			conn = mysql_init(0);
			conn = mysql_real_connect(conn, ServerIP, DbUsername, DbPassword, DbName, DbPort, NULL, 0);
			while (!conn) {
				conn = mysql_init(0);
				conn = mysql_real_connect(conn, ServerIP, DbUsername, DbPassword, DbName, DbPort, NULL, 0);
				cout << "Error connecting to Database, Reconnecting...\n";
				Sleep(3000);
			}
			cout << "Connected to Database!\n";

			//Lakukan Query
			cout << "Retrying query: " + query << endl;
			q = query.c_str();
			qState = mysql_query(conn, q);
			if (!qState) {
				res = mysql_store_result(conn);
				if (res != NULL) {
					qOutput = "";
					while (row = mysql_fetch_row(res)) {
						int j = res->field_count;
						for (int i = 0; i < j; i++) {
							string x = row[i];
							qOutput += x + '/';
						}
						qOutput += '\n';
					}
				}
			}
		} while (qState);
	}
	if (qOutput.empty()) {
		return "-1";
	}
	return qOutput;
}

void setup() {
	int choice = -1;

	cout << "\nChecking ClientID.txt...";
	fstream in("ClientID.txt", fstream::in);
	if (in.is_open()) { 
		in >> clientID;
		cout << " OK\n";
	}
	else { cout << "\nClientID.txt not found\n"; }
	in.close();

	if (clientID == NULL) {
		while (choice == -1) {
			cout << "\nIs this an New or Existing PC? (0 for Existing PC. 1 for New PC)\nChoice: ";
			cin >> choice; cin.get();
			if (choice == 0) {
				cout << "Input Client ID number: ";
				cin >> clientID; cin.get();

				if (stoi(sqlQuery("SELECT `id` FROM `clients` WHERE `id` = " + to_string(clientID))) == clientID) {}
				else {
					cout << "\nClient ID not found in the Database.\n";
					choice = -1;
				}
			}
			else if (choice == 1) {
				cout << "\nCreating an new Client ID...";
				sqlQuery("INSERT INTO `clients` (`id`, `updated?`) VALUES (NULL, '0')");
				clientID = stoi(sqlQuery("SELECT `id` FROM `clients` ORDER BY `id` DESC LIMIT 1"));
				cout << " OK\n";
				cout << "\nYour new Client ID is: " << clientID << "\n\n";
			}
			else {
				cout << "Invalid choice...\n";
				choice = -1;
			}
		}
		ofstream out("ClientID.txt");
		out << clientID;
		out.close();
	}

	cout << "Checking Client ID...";
	if (stoi(sqlQuery("SELECT `id` FROM `clients` WHERE `id` = " + to_string(clientID))) != clientID) {
		cout << "\nClient ID not found in the Database.\n";
		remove("ClientID.txt");
		clientID = NULL;
		setup();
	}
	cout << " OK\n";
}

void main() {
	cout << "[ Client ID Setup ]\n";
	cout << "Connecting to the Database...\n";
	connectToDatabase();
	setup();
	cout << "\nSetup done. Press any key to Exit...\n";
	system("pause");
}