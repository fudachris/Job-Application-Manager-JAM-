#include <iostream>
#include <stdlib.h>
#include <stdio.h>
#include <sqlite3.h>
#include <fstream>
#include <string.h>
#include <sstream>
#include <algorithm>

//open db or create if doesnt exist
sqlite3* 
getDB (void)
{
    const int STATEMENTS = 8;
    sqlite3 *db;
    char *zErrMsg = 0;
    const char *pSQL[STATEMENTS];

    int rc = sqlite3_open_v2 ("jam.db",
                              &db,
                              SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, 
                              NULL 
                             );

    if (rc != SQLITE_OK)
    {
        std::cout << "error open/create db " <<std::endl;
    }
    else
    {
        std::cout << "db open success " << std::endl;
    }

    return (db);
}

//create table to hold data or create if doesnt exist
void
createTable (sqlite3* db)
{
    const char *sqlq = "CREATE TABLE IF NOT EXISTS JOBLIST "
                       "(id INTEGER PRIMARY KEY AUTOINCREMENT,"
                       " Name TEXT," 
                       " Date TEXT,"
                       " Status TEXT,"
                       " Description VARCHAR(255) NOT NULL,"
                       " CV VARCHAR(255) NOT NULL,"
                       " Log VARCHAR(255) NOT NULL);";
       
    char *err_msg = 0;

    int rc = sqlite3_exec(db,sqlq,0,0,&err_msg); 

    if (rc != SQLITE_OK)
    {
        std::cout << "error open/create table " << err_msg <<std::endl;
    }
    else
    {
        std::cout << "table open/create success " << std::endl;
    }

}

//callback for printing table contents
static int 
callback (void *nu, int argc, char **argv, char **azColName)
{
    for (int i = 0; i < argc; i++)
    {
        printf("%s = %s\n", azColName[i], argv[i] ? argv [i] : "NULL");
    }
    
    std::cout << std::endl;
    
    return 0;
}

//print table 
void 
printTable (sqlite3 *db)
{
    const char* sqlq = "SELECT * FROM JOBLIST";
    const char* data = "Callback function called";
    char *err_msg = 0;

    int rc = sqlite3_exec (db, sqlq, callback, (void*) data, &err_msg);  
    
    if (rc != SQLITE_OK)
    {
        std::cout << "error printing " << err_msg << std::endl;
    }
    else
    {
        std::cout <<"Table print successfull " << std::endl;
    }

}

//insert data into db
void 
insertData (sqlite3 *db)
{
    std::cin.ignore();
    std::cin.clear();

    std::string name = "";

    std::string date = "";
    std::string status = "";
    std::string description = "";
    std::string cv = "";
    std::string log = "";

    std::cout << "\n enter name " << std::endl;
    getline (std::cin,name);
    std::cout << "\n name " << name<<std::endl;
    
    std::cout << "\n enter date " << std::endl;
    getline (std::cin,date);
    std::cout << "\n date " << date<<std::endl;
    
    std::cout << "\n enter status " << std::endl;
    getline (std::cin,status);
    std::cout << "\n status " << status<<std::endl;
    
    std::cout << "\n enter description file location " << std::endl;
    getline (std::cin,description);
    std::cout << "\n description " <<description<<std::endl;
    
    std::cout << "\n enter cv file location " << std::endl;
    getline (std::cin,cv);
    std::cout << "\n cv " <<cv<<std::endl;
    
    std::cout << "\n enter log file location " << std::endl;
    getline (std::cin,log);
    std::cout << "\n log " << log<<std::endl;

    const char* sqlq = NULL;

    std::string tmp = "INSERT INTO JOBLIST (Name,"
                      "Date,"
                      "Status,"
                      "Description,"
                      "CV,"
                      "Log"
                      ")" 
                      " VALUES ('"+name+"',"
                                "'"+date+"',"
                                "'"+status+"',"
                                +description+","
                                +cv+","
                                +log+
                              ");";

    sqlq = tmp.c_str();        

    std::cout << "\n sqlq " << sqlq << std::endl;    

    sqlite3_stmt *stmt; 

    sqlite3_prepare_v2(db, sqlq, strlen(sqlq), &stmt, NULL);                              

    int rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE) {
            printf("ERROR inserting data: %s\n", sqlite3_errmsg(db));
    }

    sqlite3_finalize(stmt);
}

//convert char to string with proper formatting for gnome-open
std::string
char2str (const unsigned char *data)
{

    std::string str_data(reinterpret_cast<const char*>(data));

    std::string str_tmp = "";
    std::string str_data_new = "";

    std::stringstream ssStr (str_data);

    while(getline(ssStr, str_tmp, ' '))
    {
        str_data_new +=str_tmp;
        str_data_new += "\\ ";
    }

    str_data_new = str_data_new.substr (0, str_data_new.size()-2);
    std::string sys_command = "gnome-open "+str_data_new;

    return sys_command;
}

//documents will be opened using gnome-open
void
openDocuments (sqlite3* db)
{
    int id = 0, rc =0;
    std::cout << "\n enter id " << std::endl;
    std::cin >> id;

    std::stringstream ssId;
    ssId << id;

    const char* sqlq = NULL;

    std::string str_id [3] = {"Description","CV","Log"}; 

    for (int j = 0 ; j < 3; j++)
    {
        std::string tmp = "SELECT "+str_id[j]+" FROM JOBLIST WHERE id="+ssId.str();
        sqlq = tmp.c_str();

        sqlite3_stmt *stmt; 

        sqlite3_prepare_v2(db, sqlq, strlen(sqlq), &stmt, NULL);      

        const unsigned char *data;
        std::string sys_command = "";
        
        while (SQLITE_ROW == (rc = sqlite3_step(stmt))) {
            for(int i = 0; i < sqlite3_column_count(stmt); i++) {
                data = sqlite3_column_text(stmt, i); // text in ith column
                sys_command = char2str(data);
            }                                                                                                                                                                          
            std::cout << std::endl;
        }

        system(sys_command.c_str());

        if (rc != SQLITE_DONE) {
            printf("ERROR selecting data: %s\n", sqlite3_errmsg(db));
        }

        sqlite3_finalize(stmt);
    }
}

//update field by id
void
updateField (sqlite3 *db)
{
    int id = 0;
    std::stringstream ssId;

    std::cout << " enter id to update " << std::endl;
    std::cin >> id;

    ssId << id;

    std::cin.ignore();
    std::cin.clear();

    std::string field_name = "";
    std::cout << "enter the field name you want to update " << std::endl;
    getline (std::cin, field_name);
    
    std::string new_val = "";
    std::cout << " enter new  value " << std::endl;
    getline (std::cin, new_val);

    std::string new_val_fixed = "'" + new_val + "'";

    std::string tmp = "UPDATE JOBLIST set "+field_name+" = "+new_val_fixed+" where id = "+ ssId.str()+";";
    const char * sqlq = tmp.c_str();

    char* err_msg = 0;

    int rc = sqlite3_exec(db, sqlq,0,0, &err_msg);

    if (rc != SQLITE_OK)
    {
        std::cout <<" update error " << err_msg << std::endl;
    }
    else
    {
        std::cout << " update successful " << std::endl;
    }

}

//function to setup transcript location or get transcript if already exists
void
getTranscript (void)
{
    //check if file containing location to transcript exits
    //if file doesnt exist/empty, create it and append transcript location
    std::ifstream f("trans.txt");
    if(!f.good())
    {
        std::cin.ignore();
        std::cin.clear();

        std::ofstream outfile ("trans.txt");

        std::cout << "\n new file - enter transcript location " << std::endl;
        
        std::string location = "";

        getline(std::cin,location);

        outfile << location;
    }
    else if (f.peek() == std::ifstream::traits_type::eof())
    {
        std::cin.ignore();
        std::cin.clear();

        std::ofstream outfile ("trans.txt",std::ios::app);

        std::cout << "\n empty file - enter transcript location " << std::endl;
        
        std::string location = "";

        getline(std::cin,location);

        outfile << location;
    }
    else
    {
        std::string line;
        getline(f,line);
        line = line.substr (1,line.size());
        std::string sys_command = char2str(reinterpret_cast<const unsigned char*>(line.c_str()));
        sys_command = sys_command.substr(0,sys_command.size()-1);
        system(sys_command.c_str());
    }
}


//menu function
int
dispMenu (void)
{
    std::cout << "Menu: " << std::endl;
    std::cout << "1 - Print table " << std::endl;
    std::cout << "2 - Open related documents by id " << std::endl;
    std::cout << "3 - Update information by id " << std::endl;
    std::cout << "4 - Inset data into table " << std::endl;
    std::cout << "5 - Get transcript " << std::endl;
    std::cout << "6 - exit " <<std::endl;
    
    int m = 0;
    std::cin >> m;
    return m;
}

int main ()
{
    sqlite3 *db = getDB();
    createTable(db);

    int m = 0;

    while (true)
    {
        m = dispMenu ();

        if ( m == 1 )
        {
            printTable(db); 
        }
        else if ( m == 2 )
        {
            openDocuments (db);
        }
        else if ( m == 3 )
        {
            updateField (db);
        }
        else if ( m == 4 )
        {
            insertData (db);
        }
        else if ( m == 5 )
        {
            getTranscript ( );
        }
        else if ( m == 6 )
        {
            return 0;
        }

    }

    return 0;
}
