#include <string>
using namespace std;

/*
	2.3->2.4
	´´½¨
		dtu_receive_buffer
		dtu_send_buffer
*/

#define E_BEOPDB_VERSION	5

//#define E_DROPDB_BEOP	"DROP DATABASE if exists beopdata"
#define E_CREATEDB_BEOP "CREATE DATABASE if not exists beopdata;"


#define E_CREATEDB_DOM_LOG "CREATE DATABASE if not exists domlog;"

//#define E_DROPTB_BEOPINFO "DROP TABLE IF EXISTS beopdata.`beopinfo`;"
#define E_CREATETB_BEOPINFO "CREATE TABLE beopdata.`beopinfo` (\
							`infoname` varchar(45) NOT NULL DEFAULT '',\
							`incocontent` varchar(512) NOT NULL DEFAULT ''\
							) ENGINE=InnoDB DEFAULT CHARSET=utf8;"
#define E_INSERTTB_BEOPINFO	"INSERT INTO beopdata.`beopinfo` (`infoname`,`incocontent`) VALUES \
							('version','2.3');"
#define E_UPDATETB_BEOPINFO	"UPDATE beopdata.`beopinfo` set `incocontent` = '4' where `infoname` = 'version';"


//#define	E_DROPTB_LOG "DROP TABLE IF EXISTS beopdata.`log`;"
#define E_CREATETB_LOG "CREATE TABLE beopdata.`log` (\
							`time` timestamp NOT NULL DEFAULT '0000-00-00 00:00:00',\
							`loginfo` varchar(20000) NOT NULL DEFAULT ''\
							) ENGINE=InnoDB DEFAULT CHARSET=utf8;"


//#define E_DROPTB_OPERATION_RECORD "DROP TABLE IF EXISTS beopdata.`operation_record`;"
#define E_CREATETB_OPERATION_RECORD "CREATE TABLE beopdata.`operation_record` (\
									`RecordTime` timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP,\
									`user` varchar(128) NOT NULL DEFAULT '',\
									`OptRemark` varchar(256) NOT NULL DEFAULT ''\
									) ENGINE=InnoDB DEFAULT CHARSET=utf8;"

//#define E_DROPTB_POINT_VALUE_BUFFER		"DROP TABLE IF EXISTS beopdata.`point_value_buffer`;"
#define E_CREATETB_POINT_VALUE_BUFFER   "CREATE TABLE beopdata.`point_value_buffer` (\
											`time` timestamp NOT NULL DEFAULT '0000-00-00 00:00:00',\
											`pointname` varchar(255) NOT NULL DEFAULT '0',\
											`pointvalue` varchar(20000) NOT NULL DEFAULT '0'\
											) ENGINE=InnoDB DEFAULT CHARSET=utf8;"

//#define E_DROPTB_REALTIMEDATA_INPUT	"DROP TABLE IF EXISTS beopdata.`realtimedata_input`;"
#define E_CREATETB_REALTIMEDATA_INPUT	"CREATE TABLE beopdata.`realtimedata_input` (\
											`time` timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,\
											`pointname` varchar(255) NOT NULL DEFAULT '',\
											`pointvalue` varchar(20000) NOT NULL DEFAULT '',\
											PRIMARY KEY (`pointname`)\
											) ENGINE=InnoDB DEFAULT CHARSET=utf8;"

//#define E_DROPTB_REALTIMEDATA_OUTPUT "DROP TABLE IF EXISTS beopdata.`realtimedata_output`;"
#define E_CREATETB_REALTIMEDATA_OUTPUT "CREATE TABLE beopdata.`realtimedata_output` (\
											`pointname` varchar(255) NOT NULL DEFAULT '',\
											`pointvalue` varchar(20000) NOT NULL DEFAULT '',\
											PRIMARY KEY (`pointname`)\
											) ENGINE=MEMORY DEFAULT CHARSET=utf8;"

#define E_UPGRADE_REALTIMEDATA_OUTPUT "alter table beopdata.`realtimedata_output`  modify column pointvalue varchar(20000)"
#define E_UPGRADE_REALTIMEDATA_INPUT "alter table beopdata.`realtimedata_input`  modify column pointvalue varchar(20000)"

#define E_CREATETB_REALTIMEDATA_INPUT_MODBUS_EQUIPMENT	"CREATE TABLE IF NOT EXISTS beopdata.`realtimedata_input_modbus_equipment` (\
`time` timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,\
`pointname` varchar(255) NOT NULL DEFAULT '',\
`pointvalue` varchar(2560) NOT NULL DEFAULT '',\
PRIMARY KEY (`pointname`)\
) ENGINE=MEMORY DEFAULT CHARSET=utf8;"

#define E_CREATETB_REALTIMEDATA_OUTPUT_MODBUS_EQUIPMENT "CREATE TABLE IF NOT EXISTS beopdata.`realtimedata_output_modbus_equipment` (\
`pointname` varchar(255) NOT NULL DEFAULT '',\
`pointvalue` varchar(2560) NOT NULL DEFAULT '',\
PRIMARY KEY (`pointname`)\
) ENGINE=MEMORY DEFAULT CHARSET=utf8;"


#define E_CREATETB_REALTIMEDATA_INPUT_PERSAGY_CONTROLLER	"CREATE TABLE IF NOT EXISTS beopdata.`realtimedata_input_persagy_controller` (\
`time` timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,\
`pointname` varchar(255) NOT NULL DEFAULT '',\
`pointvalue` varchar(2560) NOT NULL DEFAULT '',\
PRIMARY KEY (`pointname`)\
) ENGINE=MEMORY DEFAULT CHARSET=utf8;"

#define E_CREATETB_REALTIMEDATA_OUTPUT_PERSAGY_CONTROLLER "CREATE TABLE IF NOT EXISTS beopdata.`realtimedata_output_persagy_controller` (\
`pointname` varchar(255) NOT NULL DEFAULT '',\
`pointvalue` varchar(2560) NOT NULL DEFAULT '',\
PRIMARY KEY (`pointname`)\
) ENGINE=MEMORY DEFAULT CHARSET=utf8;"

#define E_CREATETB_REALTIMEDATA_INPUT_THIRDPARTY	"CREATE TABLE IF NOT EXISTS beopdata.`realtimedata_input_thirdparty` (\
`time` timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,\
`pointname` varchar(255) NOT NULL DEFAULT '',\
`pointvalue` varchar(2560) NOT NULL DEFAULT '',\
PRIMARY KEY (`pointname`)\
) ENGINE=MEMORY DEFAULT CHARSET=utf8;"

#define E_CREATETB_REALTIMEDATA_OUTPUT_THIRDPARTY "CREATE TABLE IF NOT EXISTS beopdata.`realtimedata_output_thirdparty` (\
`pointname` varchar(255) NOT NULL DEFAULT '',\
`pointvalue` varchar(2560) NOT NULL DEFAULT '',\
PRIMARY KEY (`pointname`)\
) ENGINE=MEMORY DEFAULT CHARSET=utf8;"

//#define E_DROPTB_UNIT01 "DROP TABLE IF EXISTS beopdata.`unit01`;"
#define E_CREATETB_UNIT01 "CREATE TABLE IF NOT EXISTS beopdata.`unit01` (\
								`unitproperty01` text,\
								`unitproperty02` text,\
								`unitproperty03` text,\
								`unitproperty04` text,\
								`unitproperty05` text,\
								`unitproperty06` text,\
								`unitproperty07` text,\
								`unitproperty08` text,\
								`unitproperty09` text,\
								`unitproperty10` text,\
								`unitproperty11` text,\
								`unitproperty12` text,\
								`unitproperty13` text,\
								`unitproperty14` text,\
								`unitproperty15` text\
								) ENGINE=InnoDB DEFAULT CHARSET=utf8;"

//#define E_DROPTB_UNIT02 "DROP TABLE IF EXISTS beopdata.`unit02`;"
#define E_CREATETB_UNIT02	"CREATE TABLE IF NOT EXISTS beopdata.`unit02` (\
									`unitproperty01` text,\
									`unitproperty02` text,\
									`unitproperty03` text,\
									`unitproperty04` text,\
									`unitproperty05` text,\
									`unitproperty06` text,\
									`unitproperty07` text,\
									`unitproperty08` text,\
									`unitproperty09` text,\
									`unitproperty10` text,\
									`unitproperty11` text,\
									`unitproperty12` text,\
									`unitproperty13` text,\
									`unitproperty14` text,\
									`unitproperty15` text\
									) ENGINE=InnoDB DEFAULT CHARSET=utf8;"

//#define E_DROPTB_UNIT03 "DROP TABLE IF EXISTS beopdata.`unit03`;"
#define E_CREATETB_UNIT03	"CREATE TABLE IF NOT EXISTS beopdata.`unit03` (\
									`unitproperty01` text,\
									`unitproperty02` text,\
									`unitproperty03` text,\
									`unitproperty04` text,\
									`unitproperty05` text,\
									`unitproperty06` text,\
									`unitproperty07` text,\
									`unitproperty08` text,\
									`unitproperty09` text,\
									`unitproperty10` text,\
									`unitproperty11` text,\
									`unitproperty12` text,\
									`unitproperty13` text,\
									`unitproperty14` text,\
									`unitproperty15` text\
									) ENGINE=InnoDB DEFAULT CHARSET=utf8;"


//#define E_DROPTB_UNIT04 "DROP TABLE IF EXISTS beopdata.`unit04`;"
#define E_CREATETB_UNIT04	"CREATE TABLE IF NOT EXISTS beopdata.`unit04` (\
									`unitproperty01` text,\
									`unitproperty02` text,\
									`unitproperty03` text,\
									`unitproperty04` text,\
									`unitproperty05` text,\
									`unitproperty06` text,\
									`unitproperty07` text,\
									`unitproperty08` text,\
									`unitproperty09` text,\
									`unitproperty10` text,\
									`unitproperty11` text,\
									`unitproperty12` text,\
									`unitproperty13` text,\
									`unitproperty14` text,\
									`unitproperty15` text\
									) ENGINE=InnoDB DEFAULT CHARSET=utf8;"

//#define E_DROPTB_UNIT05 "DROP TABLE IF EXISTS beopdata.`unit05`;"
#define E_CREATETB_UNIT05	"CREATE TABLE IF NOT EXISTS beopdata.`unit05` (\
									`unitproperty01` text,\
									`unitproperty02` text,\
									`unitproperty03` text,\
									`unitproperty04` text,\
									`unitproperty05` text,\
									`unitproperty06` text,\
									`unitproperty07` text,\
									`unitproperty08` text,\
									`unitproperty09` text,\
									`unitproperty10` text,\
									`unitproperty11` text,\
									`unitproperty12` text,\
									`unitproperty13` text,\
									`unitproperty14` text,\
									`unitproperty15` text\
									) ENGINE=InnoDB DEFAULT CHARSET=utf8;"
							
//#define E_DROPTB_UNIT06 "DROP TABLE IF EXISTS beopdata.`unit06`;"
#define E_CREATETB_UNIT06	"CREATE TABLE IF NOT EXISTS beopdata.`unit06` (\
									`unitproperty01` text,\
									`unitproperty02` text,\
									`unitproperty03` text,\
									`unitproperty04` text,\
									`unitproperty05` text,\
									`unitproperty06` text,\
									`unitproperty07` text,\
									`unitproperty08` text,\
									`unitproperty09` text,\
									`unitproperty10` text,\
									`unitproperty11` text,\
									`unitproperty12` text,\
									`unitproperty13` text,\
									`unitproperty14` text,\
									`unitproperty15` text\
									) ENGINE=InnoDB DEFAULT CHARSET=utf8;"

//#define E_DROPTB_UNIT07 "DROP TABLE IF EXISTS beopdata.`unit07`;"
#define E_CREATETB_UNIT07	"CREATE TABLE IF NOT EXISTS beopdata.`unit07` (\
									`unitproperty01` text,\
									`unitproperty02` text,\
									`unitproperty03` text,\
									`unitproperty04` text,\
									`unitproperty05` text,\
									`unitproperty06` text,\
									`unitproperty07` text,\
									`unitproperty08` text,\
									`unitproperty09` text,\
									`unitproperty10` text,\
									`unitproperty11` text,\
									`unitproperty12` text,\
									`unitproperty13` text,\
									`unitproperty14` text,\
									`unitproperty15` text\
									) ENGINE=InnoDB DEFAULT CHARSET=utf8;"

//#define E_DROPTB_UNIT08 "DROP TABLE IF EXISTS beopdata.`unit08`;"
#define E_CREATETB_UNIT08	"CREATE TABLE IF NOT EXISTS beopdata.`unit08` (\
									`unitproperty01` text,\
									`unitproperty02` text,\
									`unitproperty03` text,\
									`unitproperty04` text,\
									`unitproperty05` text,\
									`unitproperty06` text,\
									`unitproperty07` text,\
									`unitproperty08` text,\
									`unitproperty09` text,\
									`unitproperty10` text,\
									`unitproperty11` text,\
									`unitproperty12` text,\
									`unitproperty13` text,\
									`unitproperty14` text,\
									`unitproperty15` text\
									) ENGINE=InnoDB DEFAULT CHARSET=utf8;"

//#define E_DROPTB_UNIT09 "DROP TABLE IF EXISTS beopdata.`unit09`;"
#define E_CREATETB_UNIT09	"CREATE TABLE IF NOT EXISTS beopdata.`unit09` (\
									`unitproperty01` text,\
									`unitproperty02` text,\
									`unitproperty03` text,\
									`unitproperty04` text,\
									`unitproperty05` text,\
									`unitproperty06` text,\
									`unitproperty07` text,\
									`unitproperty08` text,\
									`unitproperty09` text,\
									`unitproperty10` text,\
									`unitproperty11` text,\
									`unitproperty12` text,\
									`unitproperty13` text,\
									`unitproperty14` text,\
									`unitproperty15` text\
									) ENGINE=InnoDB DEFAULT CHARSET=utf8;"

//#define E_DROPTB_UNIT10 "DROP TABLE IF EXISTS beopdata.`unit10`;"
#define E_CREATETB_UNIT10		"CREATE TABLE IF NOT EXISTS beopdata.`unit10` (\
										`unitproperty01` text,\
										`unitproperty02` text,\
										`unitproperty03` text,\
										`unitproperty04` text,\
										`unitproperty05` text,\
										`unitproperty06` text,\
										`unitproperty07` text,\
										`unitproperty08` text,\
										`unitproperty09` text,\
										`unitproperty10` text,\
										`unitproperty11` text,\
										`unitproperty12` text,\
										`unitproperty13` text,\
										`unitproperty14` text,\
										`unitproperty15` text\
										) ENGINE=InnoDB DEFAULT CHARSET=utf8;"

//#define E_DROPTB_USERLIST_ONLINE	"DROP TABLE IF EXISTS beopdata.`userlist_online`;"
#define E_CREATETB_USERLIST_ONLINE	"CREATE TABLE IF NOT EXISTS beopdata.`userlist_online` (\
										`username` varchar(64) NOT NULL DEFAULT '',\
										`userhost` varchar(64) NOT NULL,\
										`priority` int(10) unsigned NOT NULL,\
										`usertype` varchar(64) NOT NULL,\
										`time` timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,\
										PRIMARY KEY (`username`,`usertype`)\
										) ENGINE=MEMORY DEFAULT CHARSET=utf8;"
#define E_INSERTTB_USERLIST_ONLINE	"INSERT INTO beopdata.`userlist_online` (`username`,`userhost`,`priority`,`usertype`,`time`) VALUES\
										('operator','',0,'client','2014-01-06 11:21:46');"

//#define E_DROPTB_WARNING_CONFIG	"DROP TABLE IF EXISTS beopdata.`warning_config`;"
#define E_CREATETB_WARNING_CONFIG	"CREATE TABLE IF NOT EXISTS beopdata.`warning_config` (\
										`HHEnable` int(10) unsigned DEFAULT NULL,\
										`HEnable` int(10) unsigned DEFAULT NULL,\
										`LEnable` int(10) unsigned DEFAULT NULL,\
										`LLEnable` int(10) unsigned DEFAULT NULL,\
										`HHLimit` double DEFAULT NULL,\
										`HLimit` double DEFAULT NULL,\
										`LLimit` double DEFAULT NULL,\
										`LLLimit` double DEFAULT NULL,\
										`pointname` varchar(255) NOT NULL DEFAULT '',\
										`HHwarninginfo` varchar(255) DEFAULT NULL,\
										`Hwarninginfo` varchar(255) DEFAULT NULL,\
										`Lwarninginfo` varchar(255) DEFAULT NULL,\
										`LLwarninginfo` varchar(255) DEFAULT NULL,\
										`boolwarning` int(11) DEFAULT NULL,\
										`boolwarninginfo` varchar(255) DEFAULT NULL,\
										`boolwarninglevel` int(11) DEFAULT NULL,\
										`unitproperty01` varchar(255) DEFAULT NULL,\
										`unitproperty02` varchar(255) DEFAULT NULL,\
										`unitproperty03` varchar(255) DEFAULT NULL,\
										`unitproperty04` varchar(255) DEFAULT NULL,\
										`unitproperty05` varchar(255) DEFAULT NULL\
										) ENGINE=InnoDB DEFAULT CHARSET=utf8;"

#define E_UPGRADE_WARNING_CONFIG "alter table beopdata.warning_config add column script varchar(2000), add column OfPosition varchar(255), add column  ofSystem varchar(255), \
										 add column ofDepartment varchar(255),  add column ofGroup varchar(255),  add column tag varchar(1024),  add column id INT;"
#define E_UPGRADE_WARNING_RECORD "alter table beopdata.warningrecord add column id INT, add column ruleId INT, \
                             add column OfPosition varchar(255), add column  ofSystem varchar(255), \
										 add column ofDepartment varchar(255),  add column ofGroup varchar(255),  add column tag varchar(1024);"
#define E_UPGRADE_WARNING_RECORD2 "alter table beopdata.warningrecord add column infodetail varchar(2000),  add column unitproperty01 varchar(1024),  add column unitproperty02 varchar(1024),  add column unitproperty03 varchar(1024),  add column unitproperty04 varchar(1024),  add column unitproperty05 varchar(1024);"
#define E_UPGRADE_WARNING_CONFIG2 "alter table beopdata.warning_config add column infodetail varchar(2000);"


//#define E_DROPTB_WARNING_RECORD	"DROP TABLE IF EXISTS beopdata.`warningrecord`;"
#define E_CREATETB_WARNING_RECORD	"CREATE TABLE IF NOT EXISTS beopdata.`warningrecord` (\
									`time` timestamp NOT NULL DEFAULT '2000-01-01 00:00:00',\
									`code` int(10) unsigned NOT NULL DEFAULT '0',\
									`info` varchar(1024) NOT NULL DEFAULT '',\
									`level` int(10) unsigned NOT NULL DEFAULT '0',\
									`endtime` timestamp NOT NULL DEFAULT '2000-01-01 00:00:00',\
									`confirmed` int(10) unsigned NOT NULL DEFAULT '0',\
									`confirmeduser` varchar(2000) NOT NULL DEFAULT '',\
									`bindpointname` varchar(255) DEFAULT NULL\
									) ENGINE=InnoDB DEFAULT CHARSET=utf8;"

//#define E_DROPTB_FILESTORAGE	"DROP TABLE IF EXISTS `beopdata`.`filestorage`;"
#define E_CREATETB_FILESTORAGE		"CREATE TABLE IF NOT EXISTS  `beopdata`.`filestorage` (\
									`fileid` varchar(255) NOT NULL,\
									`filename` varchar(255) NOT NULL DEFAULT '',\
									`filedescription` varchar(255) DEFAULT NULL,\
									`fileupdatetime` timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,\
									`reserve01` varchar(255) DEFAULT NULL,\
									`reserve02` varchar(255) DEFAULT NULL,\
									`reserve03` varchar(255) DEFAULT NULL,\
									`reserve04` varchar(255) DEFAULT NULL,\
									`reserve05` varchar(255) DEFAULT NULL,\
									`fileblob` LONGBLOB DEFAULT NULL\
									) ENGINE=InnoDB DEFAULT CHARSET=utf8;"

//#define E_DROPTB_BEOPUSER	"DROP TABLE IF EXISTS `beopdata`.`beopuser`;"
#define E_CREATETB_BEOPUSER		"CREATE TABLE IF NOT EXISTS  `beopdata`.`beopuser` (\
									`userid` varchar(255) DEFAULT NULL,\
										`username` varchar(255) DEFAULT NULL,\
										`userpwd` varchar(255) DEFAULT NULL,\
										`userfullname` varchar(255) DEFAULT NULL,\
										`usersex` varchar(255) DEFAULT NULL,\
										`usermobile` varchar(255) DEFAULT NULL,\
										`useremail` varchar(255) DEFAULT NULL,\
										`usercreatedt` varchar(255) DEFAULT NULL,\
										`userstatus` varchar(255) DEFAULT NULL,\
										`userpic` LONGBLOB DEFAULT NULL,\
										`userofrole` varchar(255) DEFAULT NULL,\
										`unitproperty01` varchar(255) DEFAULT NULL,\
										`unitproperty02` varchar(255) DEFAULT NULL,\
										`unitproperty03` varchar(255) DEFAULT NULL,\
										`unitproperty04` varchar(255) DEFAULT NULL,\
										`unitproperty05` varchar(255) DEFAULT NULL\
										) ENGINE=InnoDB DEFAULT CHARSET=utf8;"

//#define E_DROPTB_BEOPROLE	"DROP TABLE IF EXISTS `beopdata`.`beoprole`;"
#define E_CREATETB_BEOPROLE		"CREATE TABLE IF NOT EXISTS  `beopdata`.`beoprole` (\
								`roleid` varchar(255) DEFAULT NULL,\
									`rolename` varchar(255) DEFAULT NULL,\
									`roledesc` varchar(255) DEFAULT NULL,\
									`rolecreatedt` varchar(255) DEFAULT NULL,\
									`rolestatus` varchar(255) DEFAULT NULL,\
									`rolerightbasic` TEXT DEFAULT NULL,\
									`rolerightpage` TEXT DEFAULT NULL,\
									`rolerightwritepoint` TEXT DEFAULT NULL,\
									`roleright1` TEXT DEFAULT NULL,\
									`roleright2` TEXT DEFAULT NULL,\
									`roleright3` TEXT DEFAULT NULL,\
									`roleright4` TEXT DEFAULT NULL,\
									`roleright5` TEXT DEFAULT NULL,\
									`unitproperty01` varchar(255) DEFAULT NULL,\
									`unitproperty02` varchar(255) DEFAULT NULL,\
									`unitproperty03` varchar(255) DEFAULT NULL,\
									`unitproperty04` varchar(255) DEFAULT NULL,\
									`unitproperty05` varchar(255) DEFAULT NULL\
									) ENGINE=InnoDB DEFAULT CHARSET=utf8;"


//#define E_DROPTB_BEOPUSER	"DROP TABLE IF EXISTS `beopdata`.`beopuser`;"
#define E_INITUSER_BEOPUSER		"insert into beopdata.`beopuser` (`userid`, `username`, `userpwd`, `userofrole`) values(0,'admin','111',3)"

//#define E_DROPTB_DTU_SEND_BUFFER	"DROP TABLE IF EXISTS `beopdata`.`dtu_send_buffer`;"
#define E_CREATETB_DTU_SEND_BUFFER		"CREATE TABLE IF NOT EXISTS  `beopdata`.`dtu_send_buffer` (\
										`id` int(10) unsigned NOT NULL AUTO_INCREMENT,\
											`time` datetime NOT NULL,\
											`filename` varchar(256) DEFAULT NULL,\
											`subtype` int(10) unsigned NOT NULL DEFAULT '0',\
											`trycount` int(10) unsigned NOT NULL DEFAULT '0',\
											`content` text,\
											PRIMARY KEY (`id`)\
											) ENGINE=InnoDB AUTO_INCREMENT=49 DEFAULT CHARSET=utf8;"

//#define E_DROPTB_DTU_RECEIVE_BUFFER	"DROP TABLE IF EXISTS `beopdata`.`dtu_receive_buffer`;"
#define E_CREATETB_DTU_RECEIVE_BUFFER		"CREATE TABLE IF NOT EXISTS  `beopdata`.`dtu_receive_buffer` (\
											`id` int(10) unsigned NOT NULL AUTO_INCREMENT,\
												`time` datetime NOT NULL,\
												`type` int(10) unsigned NOT NULL DEFAULT '0',\
												`content` text,\
												PRIMARY KEY (`id`)\
												) ENGINE=InnoDB AUTO_INCREMENT=49 DEFAULT CHARSET=utf8;"

//#define E_DROPTB_DTU_UPDATE_BUFFER	"DROP TABLE IF EXISTS `beopdata`.`dtu_update_buffer`;"
#define E_CREATETB_DTU_UPDATE_BUFFER		"CREATE TABLE IF NOT EXISTS  `beopdata`.`dtu_update_buffer` (\
											`time` datetime NOT NULL,\
												`filename` varchar(255) DEFAULT NULL,\
												`cmdid` varchar(255) DEFAULT NULL\
												) ENGINE=InnoDB DEFAULT CHARSET=utf8;"


#define E_INSERT_VALUE_UNIT01_DEFAULT		"INSERT INTO  `beopdata`.`unit01` (unitproperty01, unitproperty02) values('storehistory', '1')"




#define E_CREATETB_LOGIC_RECORD	"CREATE TABLE if not exists beopdata.`logic_output_point_record` (\
									`logicname` text,\
									`pointtime` text,\
									`pointname` text,\
									`pointvalue` text,\
									`unitproperty05` text,\
									`unitproperty06` text,\
									`unitproperty07` text,\
									`unitproperty08` text,\
									`unitproperty09` text,\
									`unitproperty10` text,\
									`unitproperty11` text,\
									`unitproperty12` text,\
									`unitproperty13` text,\
									`unitproperty14` text,\
									`unitproperty15` text\
									) ENGINE=InnoDB DEFAULT CHARSET=utf8;"



#define E_CREATETB_REPORT	"CREATE TABLE if not exists beopdata.`report_history` (\
									`id` INT(10) unsigned NOT NULL,\
									`name` varchar(255),\
									`description` varchar(1024),\
									`gentime` timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP,\
									`filesize` INT(10),\
									`author` varchar(255),\
									`url` varchar(655),\
									`reportTimeFrom` timestamp,\
									`reportTimeTo` timestamp,\
									`reportTimeType` INT(10),\
									`unitproperty01` text,\
									`unitproperty02` text,\
									`unitproperty03` text,\
									`unitproperty04` text,\
									`unitproperty05` text,\
									PRIMARY KEY (`id`)\
									) ENGINE=InnoDB DEFAULT CHARSET=utf8;"


#define E_CREATETB_SYSLOGIC		"CREATE TABLE  if not exists `beopdata`.`sys_logic` (\
								`name` varchar(255) DEFAULT NULL,\
								`author` varchar(255) DEFAULT NULL,\
								`version` varchar(255) DEFAULT NULL,\
								`group_name` varchar(255) DEFAULT NULL,\
									`order_index` INT(10),\
									`description` varchar(255) DEFAULT NULL,\
									`param_input` TEXT DEFAULT NULL,\
									`param_output` TEXT DEFAULT NULL,\
									`unitproperty01` varchar(255) DEFAULT NULL,\
									`unitproperty02` varchar(255) DEFAULT NULL,\
									`unitproperty03` varchar(255) DEFAULT NULL,\
									`unitproperty04` varchar(255) DEFAULT NULL,\
									`unitproperty05` varchar(255) DEFAULT NULL\
									) ENGINE=InnoDB DEFAULT CHARSET=utf8;"


#define E_CREATETB_REALTIMEDATA_INPUT_OPCUA	"CREATE TABLE IF NOT EXISTS beopdata.`realtimedata_input_opcua` (\
`time` timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,\
`pointname` varchar(255) NOT NULL DEFAULT '',\
`pointvalue` varchar(2560) NOT NULL DEFAULT '',\
PRIMARY KEY (`pointname`)\
) ENGINE=MEMORY DEFAULT CHARSET=utf8;"

#define E_CREATETB_REALTIMEDATA_OUTPUT_OPCUA "CREATE TABLE IF NOT EXISTS beopdata.`realtimedata_output_opcua` (\
`pointname` varchar(255) NOT NULL DEFAULT '',\
`pointvalue` varchar(2560) NOT NULL DEFAULT '',\
PRIMARY KEY (`pointname`)\
) ENGINE=MEMORY DEFAULT CHARSET=utf8;"


#define E_CREATETB_REALTIMEDATA_INPUT_OBIX	"CREATE TABLE IF NOT EXISTS beopdata.`realtimedata_input_obix` (\
`time` timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,\
`pointname` varchar(255) NOT NULL DEFAULT '',\
`pointvalue` varchar(2560) NOT NULL DEFAULT '',\
PRIMARY KEY (`pointname`)\
) ENGINE=MEMORY DEFAULT CHARSET=utf8;"

#define E_CREATETB_REALTIMEDATA_OUTPUT_OBIX "CREATE TABLE IF NOT EXISTS beopdata.`realtimedata_output_obix` (\
`pointname` varchar(255) NOT NULL DEFAULT '',\
`pointvalue` varchar(2560) NOT NULL DEFAULT '',\
PRIMARY KEY (`pointname`)\
) ENGINE=MEMORY DEFAULT CHARSET=utf8;"



#define E_CREATETB_REALTIMEDATA_INPUT_LOGIX	"CREATE TABLE IF NOT EXISTS beopdata.`realtimedata_input_logix` (\
`time` timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,\
`pointname` varchar(255) NOT NULL DEFAULT '',\
`pointvalue` varchar(2560) NOT NULL DEFAULT '',\
PRIMARY KEY (`pointname`)\
) ENGINE=MEMORY DEFAULT CHARSET=utf8;"

#define E_CREATETB_REALTIMEDATA_OUTPUT_LOGIX "CREATE TABLE IF NOT EXISTS beopdata.`realtimedata_output_logix` (\
`pointname` varchar(255) NOT NULL DEFAULT '',\
`pointvalue` varchar(2560) NOT NULL DEFAULT '',\
PRIMARY KEY (`pointname`)\
) ENGINE=MEMORY DEFAULT CHARSET=utf8;"




#define E_CREATETB_REALTIMEDATA_INPUT_KNX	"CREATE TABLE IF NOT EXISTS beopdata.`realtimedata_input_knx` (\
`time` timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,\
`pointname` varchar(255) NOT NULL DEFAULT '',\
`pointvalue` varchar(2560) NOT NULL DEFAULT '',\
PRIMARY KEY (`pointname`)\
) ENGINE=MEMORY DEFAULT CHARSET=utf8;"

#define E_CREATETB_REALTIMEDATA_OUTPUT_KNX "CREATE TABLE IF NOT EXISTS beopdata.`realtimedata_output_knx` (\
`pointname` varchar(255) NOT NULL DEFAULT '',\
`pointvalue` varchar(2560) NOT NULL DEFAULT '',\
PRIMARY KEY (`pointname`)\
) ENGINE=MEMORY DEFAULT CHARSET=utf8;"


#define E_CREATETB_ENV	"CREATE TABLE IF NOT EXISTS beopdata.`env` (\
											`id` INT(10),\
											`type` INT(10),\
											`enabled` INT(10),\
											`name` varchar(255) NOT NULL DEFAULT 'unnamed',\
											`description` varchar(2550) NOT NULL DEFAULT '',\
											`tags` varchar(2550) NOT NULL DEFAULT '',\
											`creator` varchar(255) NOT NULL DEFAULT '',\
											`createtime`  timestamp NOT NULL DEFAULT '2000-01-01 00:00:00',\
											PRIMARY KEY (`id`)\
											) ENGINE=InnoDB DEFAULT CHARSET=utf8;"

#define E_CREATETB_ENV_DETAIL	"CREATE TABLE IF NOT EXISTS beopdata.`env_detail` (\
											`envid` INT(10),\
											`pointname` varchar(255) NOT NULL DEFAULT '',\
											`pointvalue` varchar(20000) NOT NULL DEFAULT '',\
											PRIMARY KEY (`envid`, `pointname`)\
											) ENGINE=InnoDB DEFAULT CHARSET=utf8;"




#define E_CREATETB_MODE	"CREATE TABLE IF NOT EXISTS beopdata.`mode` (\
											`id` INT(10),\
											`type` INT(10),\
											`enabled` INT(10),\
											`name` varchar(255) NOT NULL DEFAULT 'unnamed',\
											`description` varchar(2550) NOT NULL DEFAULT '',\
											`tags` varchar(2550) NOT NULL DEFAULT '',\
											`creator` varchar(255) NOT NULL DEFAULT '',\
											`createtime`  timestamp NOT NULL DEFAULT '2000-01-01 00:00:00',\
											PRIMARY KEY (`id`)\
											) ENGINE=InnoDB DEFAULT CHARSET=utf8;"

#define E_CREATETB_MODE_DETAIL	"CREATE TABLE IF NOT EXISTS beopdata.`mode_detail` (\
											`modeid` INT(10),\
											`triggerTime` varchar(64) NOT NULL DEFAULT '',\
											`triggerTimeType` INT(10) NOT NULL DEFAULT 0,\
											`triggerTurn` INT(10) NOT NULL DEFAULT 0,\
											`triggerEnvId` INT(10) NOT NULL DEFAULT -1,\
											`triggerPointNameList` TEXT DEFAULT NULL,\
											`triggerValue` varchar(255) NOT NULL DEFAULT '' \
											) ENGINE=InnoDB DEFAULT CHARSET=utf8;"

#define E_CREATETB_MODE_CALENDAR	"CREATE TABLE IF NOT EXISTS beopdata.`mode_calendar` (\
											`ofDate` datetime NOT NULL DEFAULT '2019-01-01 00:00:00',\
											`modeid` INT(10) NOT NULL DEFAULT -1,\
											`creator` varchar(255) NOT NULL DEFAULT '',\
											`ofSystem` INT(10) NOT NULL DEFAULT -1,\
											PRIMARY KEY (`ofDate`, `modeId`)\
											) ENGINE=InnoDB DEFAULT CHARSET=utf8;"



#define E_CREATETB_WEATHER	"CREATE TABLE IF NOT EXISTS beopdata.`weather_calendar` (\
											`ofDate` datetime NOT NULL DEFAULT '2019-01-01 00:00:00',\
											`calendar` TEXT DEFAULT NULL,\
											`forcast` TEXT DEFAULT NULL,\
											`realtime` TEXT DEFAULT NULL,\
											`rawdata` TEXT DEFAULT NULL,\
											`raw_update_time` datetime NOT NULL DEFAULT '2019-01-01 00:00:00',\
											`unitproperty01` TEXT DEFAULT NULL,\
											`unitproperty02` TEXT DEFAULT NULL,\
											`unitproperty03` TEXT DEFAULT NULL,\
											`unitproperty04` TEXT DEFAULT NULL,\
											`unitproperty05` TEXT DEFAULT NULL,\
											`unitproperty06` TEXT DEFAULT NULL,\
											`unitproperty07` TEXT DEFAULT NULL,\
											`unitproperty08` TEXT DEFAULT NULL,\
											`unitproperty09` TEXT DEFAULT NULL,\
											`unitproperty10` TEXT DEFAULT NULL,\
											PRIMARY KEY (`ofDate`)\
											) ENGINE=InnoDB DEFAULT CHARSET=utf8;"


#define E_CREATETB_REALTIMEDATA_INPUT_BACNET	"CREATE TABLE IF NOT EXISTS beopdata.`realtimedata_input_bacnetpy` (\
`time` timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,\
`pointname` varchar(255) NOT NULL DEFAULT '',\
`pointvalue` varchar(2560) NOT NULL DEFAULT '',\
PRIMARY KEY (`pointname`)\
) ENGINE=MEMORY DEFAULT CHARSET=utf8;"

#define E_CREATETB_REALTIMEDATA_OUTPUT_BACNET "CREATE TABLE IF NOT EXISTS beopdata.`realtimedata_output_bacnetpy` (\
`pointname` varchar(255) NOT NULL DEFAULT '',\
`pointvalue` varchar(2560) NOT NULL DEFAULT '',\
PRIMARY KEY (`pointname`)\
) ENGINE=MEMORY DEFAULT CHARSET=utf8;"




#define E_CREATETB_FIX	"CREATE TABLE IF NOT EXISTS beopdata.`fix` (\
											`id` INT(10) NOT NULL DEFAULT 0,\
											`title` varchar(1024) NOT NULL DEFAULT '',\
											`reportTime` datetime NOT NULL DEFAULT '2019-01-01 00:00:00',\
											`importance` INT(10),\
											`urgent` INT(10),\
											`content` TEXT DEFAULT NULL,\
											`result` INT(10),\
											`closeTime` datetime NOT NULL DEFAULT '2019-01-01 00:00:00',\
											`attachments` varchar(1024) NOT NULL DEFAULT '',\
											`ofEquipments` varchar(1024) NOT NULL DEFAULT '',\
											`reportUser` varchar(255) NOT NULL DEFAULT '',\
											`solveUser` varchar(1024) NOT NULL DEFAULT '',\
											`specy` varchar(1024) NOT NULL DEFAULT '',\
											`tags` varchar(1024) NOT NULL DEFAULT '',\
											`energyEffects` INT(10),\
											`unitproperty01` TEXT DEFAULT NULL,\
											`unitproperty02` TEXT DEFAULT NULL,\
											`unitproperty03` TEXT DEFAULT NULL,\
											`unitproperty04` TEXT DEFAULT NULL,\
											`unitproperty05` TEXT DEFAULT NULL,\
											`unitproperty06` TEXT DEFAULT NULL,\
											`unitproperty07` TEXT DEFAULT NULL,\
											`unitproperty08` TEXT DEFAULT NULL,\
											`unitproperty09` TEXT DEFAULT NULL,\
											`unitproperty10` TEXT DEFAULT NULL,\
											PRIMARY KEY (`id`)\
											) ENGINE=InnoDB DEFAULT CHARSET=utf8;"



#define E_CREATETB_PAGE_CONTAIN_FIX	"CREATE TABLE IF NOT EXISTS beopdata.`page_contain_fix` (\
											`pageId` VARCHAR(255) NOT NULL DEFAULT '',\
											`fixId` INT(10) NOT NULL DEFAULT 0,\
											`posX` INT(10),\
											`posY` INT(10),\
											`visible` INT(10),\
											`unitproperty01` TEXT DEFAULT NULL,\
											`unitproperty02` TEXT DEFAULT NULL,\
											`unitproperty03` TEXT DEFAULT NULL,\
											`unitproperty04` TEXT DEFAULT NULL,\
											`unitproperty05` TEXT DEFAULT NULL,\
											PRIMARY KEY (`pageId`, `fixId`)\
											) ENGINE=InnoDB DEFAULT CHARSET=utf8;"



#define E_CREATETB_REALTIMEDATA_INPUT_SIEMENSE_TCP	"CREATE TABLE IF NOT EXISTS beopdata.`realtimedata_input_siemense_tcp` (\
`time` timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,\
`pointname` varchar(255) NOT NULL DEFAULT '',\
`pointvalue` varchar(256) NOT NULL DEFAULT '',\
PRIMARY KEY (`pointname`)\
) ENGINE=MEMORY DEFAULT CHARSET=utf8;"

#define E_CREATETB_REALTIMEDATA_OUTPUT_SIEMENSE_TCP "CREATE TABLE IF NOT EXISTS beopdata.`realtimedata_output_siemense_tcp` (\
`pointname` varchar(255) NOT NULL DEFAULT '',\
`pointvalue` varchar(256) NOT NULL DEFAULT '',\
PRIMARY KEY (`pointname`)\
) ENGINE=MEMORY DEFAULT CHARSET=utf8;"

#define E_CREATE_USER_DOMPY "CREATE USER 'dompysite'@'%' IDENTIFIED BY 'DOM.cloud-2016';"
#define E_USER_GRANT_DOMPY "GRANT ALL ON *.* TO 'dompysite'@'%';"
#define E_USER_GRANT_DOMPY_EXTRA "grant select,insert,update,delete,create,drop on beopdata.* to dompysite@'%' identified by 'DOM.cloud-2016';"


#define E_CREATE_USER_DOMPY01 "CREATE USER 'dompysite01'@'%' IDENTIFIED BY 'DOM.cloud-2016';"
#define E_USER_GRANT_DOMPY01 "GRANT ALL ON *.* TO 'dompysite01'@'%';"
#define E_USER_GRANT_DOMPY01_EXTRA "grant select,insert,update,delete,create,drop on beopdata.* to dompysite01@'%' identified by 'DOM.cloud-2016';"

#define E_CREATE_USER_DOMPY02 "CREATE USER 'dompysite02'@'%' IDENTIFIED BY 'DOM.cloud-2016';"
#define E_USER_GRANT_DOMPY02 "GRANT ALL ON *.* TO 'dompysite02'@'%';"
#define E_USER_GRANT_DOMPY02_EXTRA "grant select,insert,update,delete,create,drop on beopdata.* to dompysite02@'%' identified by 'DOM.cloud-2016';"

#define E_CREATE_USER_DOMCORE "CREATE USER 'domcore'@'%' IDENTIFIED BY 'DOM.cloud-2016';"
#define E_USER_GRANT_DOMCORE "GRANT ALL ON *.* TO 'domcore'@'%';"
#define E_USER_GRANT_DOMCORE_EXTRA "grant select,insert,update,delete,create,drop on beopdata.* to domcore@'%' identified by 'DOM.cloud-2016';"


#define E_CREATE_USER_DOMLOGIC "CREATE USER 'domlogic'@'%' IDENTIFIED BY 'DOM.cloud-2016';"
#define E_USER_GRANT_DOMLOGIC "GRANT ALL ON *.* TO 'domlogic'@'%';"
#define E_USER_GRANT_DOMLOGIC_EXTRA "grant select,insert,update,delete,create,drop on beopdata.* to domlogic@'%' identified by 'DOM.cloud-2016';"


#define E_CREATE_USER_DOMTASK "CREATE USER 'domtask'@'%' IDENTIFIED BY 'DOM.cloud-2016';"
#define E_USER_GRANT_DOMTASK "GRANT ALL ON *.* TO 'domtask'@'%';"
#define E_USER_GRANT_DOMTASK_EXTRA "grant select,insert,update,delete,create,drop on beopdata.* to domtask@'%' identified by 'DOM.cloud-2016';"


#define E_CREATE_USER_DOMJOBS "CREATE USER 'domjobs'@'%' IDENTIFIED BY 'DOM.cloud-2016';"
#define E_USER_GRANT_DOMJOBS "GRANT ALL ON *.* TO 'domjobs'@'%';"
#define E_USER_GRANT_DOMJOBS_EXTRA "grant select,insert,update,delete,create,drop on beopdata.* to domjobs@'%' identified by 'DOM.cloud-2016';"

#define E_CREATE_USER_DOMPY_LOCALHOST "CREATE USER 'dompysite'@'localhost' IDENTIFIED BY 'DOM.cloud-2016';"
#define E_USER_GRANT_DOMPY_LOCALHOST "GRANT ALL ON *.* TO 'dompysite'@'localhost';"
#define E_USER_GRANT_DOMPY_LOCALHOST_EXTRA "grant select,insert,update,delete,create,drop on beopdata.* to dompysite@'localhost' identified by 'DOM.cloud-2016';"


#define E_CREATE_USER_DOMPY01_LOCALHOST "CREATE USER 'dompysite01'@'localhost' IDENTIFIED BY 'DOM.cloud-2016';"
#define E_USER_GRANT_DOMPY01_LOCALHOST "GRANT ALL ON *.* TO 'dompysite01'@'localhost';"
#define E_USER_GRANT_DOMPY01_LOCALHOST_EXTRA "grant select,insert,update,delete,create,drop on beopdata.* to dompysite01@'localhost' identified by 'DOM.cloud-2016';"

#define E_CREATE_USER_DOMPY02_LOCALHOST "CREATE USER 'dompysite02'@'localhost' IDENTIFIED BY 'DOM.cloud-2016';"
#define E_USER_GRANT_DOMPY02_LOCALHOST "GRANT ALL ON *.* TO 'dompysite02'@'localhost';"
#define E_USER_GRANT_DOMPY02_LOCALHOST_EXTRA "grant select,insert,update,delete,create,drop on beopdata.* to dompysite02@'localhost' identified by 'DOM.cloud-2016';"

#define E_CREATE_USER_DOMCORE_LOCALHOST "CREATE USER 'domcore'@'localhost' IDENTIFIED BY 'DOM.cloud-2016';"
#define E_USER_GRANT_DOMCORE_LOCALHOST "GRANT ALL ON *.* TO 'domcore'@'localhost';"
#define E_USER_GRANT_DOMCORE_LOCALHOST_EXTRA "grant select,insert,update,delete,create,drop on beopdata.* to domcore@'localhost' identified by 'DOM.cloud-2016';"


#define E_CREATE_USER_DOMLOGIC_LOCALHOST "CREATE USER 'domlogic'@'localhost' IDENTIFIED BY 'DOM.cloud-2016';"
#define E_USER_GRANT_DOMLOGIC_LOCALHOST "GRANT ALL ON *.* TO 'domlogic'@'localhost';"
#define E_USER_GRANT_DOMLOGIC_LOCALHOST_EXTRA "grant select,insert,update,delete,create,drop on beopdata.* to domlogic@'localhost' identified by 'DOM.cloud-2016';"


#define E_CREATE_USER_DOMTASK_LOCALHOST "CREATE USER 'domtask'@'localhost' IDENTIFIED BY 'DOM.cloud-2016';"
#define E_USER_GRANT_DOMTASK_LOCALHOST "GRANT ALL ON *.* TO 'domtask'@'localhost';"
#define E_USER_GRANT_DOMTASK_LOCALHOST_EXTRA "grant select,insert,update,delete,create,drop on beopdata.* to domtask@'localhost' identified by 'DOM.cloud-2016';"



#define E_CREATE_USER_DOMJOBS_LOCALHOST "CREATE USER 'domjobs'@'localhost' IDENTIFIED BY 'DOM.cloud-2016';"
#define E_USER_GRANT_DOMJOBS_LOCALHOST "GRANT ALL ON *.* TO 'domjobs'@'localhost';"
#define E_USER_GRANT_DOMJOBS_LOCALHOST_EXTRA "grant select,insert,update,delete,create,drop on beopdata.* to domjobs@'localhost' identified by 'DOM.cloud-2016';"

#define E_UPGRADE_MODE_DETAIL "alter table beopdata.mode_detail add column actionOnce INT(10),  add column reserve01 varchar(1024), add column reserve02 varchar(1024),add column reserve03 varchar(1024),add column reserve04 varchar(1024), add column reserve05 varchar(1024);"




#define E_CREATETB_REALTIMEDATA_INPUT_MOXA_TCP_SERVER	"CREATE TABLE IF NOT EXISTS beopdata.`realtimedata_input_moxa_tcp_server` (\
`time` timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,\
`pointname` varchar(255) NOT NULL DEFAULT '',\
`pointvalue` varchar(2560) NOT NULL DEFAULT '',\
PRIMARY KEY (`pointname`)\
) ENGINE=MEMORY DEFAULT CHARSET=utf8;"

#define E_CREATETB_REALTIMEDATA_OUTPUT_MOXA_TCP_SERVER "CREATE TABLE IF NOT EXISTS beopdata.`realtimedata_output_moxa_tcp_server` (\
`pointname` varchar(255) NOT NULL DEFAULT '',\
`pointvalue` varchar(2560) NOT NULL DEFAULT '',\
PRIMARY KEY (`pointname`)\
) ENGINE=MEMORY DEFAULT CHARSET=utf8;"




#define E_CREATETB_FDD_WORK_ORDER "CREATE TABLE IF NOT EXISTS beopdata.`fdd_work_order` (\
 `fddName` varchar(255) NOT NULL DEFAULT '',\
`opUserName` varchar(255) NOT NULL DEFAULT '',\
	`opType` int(11) NOT NULL DEFAULT '0',\
	`opContentData` varchar(1024) DEFAULT NULL,\
	`orderId` int(10) unsigned NOT NULL DEFAULT '0',\
	`modifyTime` timestamp NOT NULL DEFAULT '0000-00-00 00:00:00',\
	`detail` varchar(1024) DEFAULT NULL,\
	`status` int(11) NOT NULL DEFAULT '0',\
	`createTime` timestamp NOT NULL DEFAULT '0000-00-00 00:00:00',\
	`title` varchar(255) DEFAULT NULL,\
	PRIMARY KEY (`modifyTime`,`orderId`)\
) ENGINE=MEMORY DEFAULT CHARSET=utf8;"



#define E_CREATETB_REALTIMEDATA_INPUT_CORE_STATION	"CREATE TABLE IF NOT EXISTS beopdata.`realtimedata_input_core_station` (\
`time` timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,\
`pointname` varchar(255) NOT NULL DEFAULT '',\
`pointvalue` varchar(2560) NOT NULL DEFAULT '',\
PRIMARY KEY (`pointname`)\
) ENGINE=MEMORY DEFAULT CHARSET=utf8;"

#define E_CREATETB_REALTIMEDATA_OUTPUT_CORE_STATION "CREATE TABLE IF NOT EXISTS beopdata.`realtimedata_output_core_station` (\
`pointname` varchar(255) NOT NULL DEFAULT '',\
`pointvalue` varchar(2560) NOT NULL DEFAULT '',\
PRIMARY KEY (`pointname`)\
) ENGINE=MEMORY DEFAULT CHARSET=utf8;"


#define E_UPGRADE_FDD_WORK_ORDER "alter table beopdata.fdd_work_order add column ownUser varchar(255);"

#define E_CREATETB_EQUIP_ASSET "CREATE TABLE  IF NOT EXISTS  `beopdata`.`equip_asset` (\
`id` bigint(30) NOT NULL AUTO_INCREMENT,\
	`equip_id` bigint(30) NOT NULL,\
	`en_name` varchar(255) NOT NULL,\
	`param_value` varchar(255) NOT NULL,\
	`project_id` bigint(30) NOT NULL,\
	PRIMARY KEY (`id`)\
	) ENGINE=InnoDB AUTO_INCREMENT=2980 DEFAULT CHARSET=utf8;"

#define E_CREATETB_EQUIP_ASSET_FILE "CREATE TABLE  IF NOT EXISTS  `beopdata`.`equip_asset_file` (\
`id` bigint(30) NOT NULL AUTO_INCREMENT,\
	`fileName` varchar(255) NOT NULL,\
	`url` varchar(255) NOT NULL,\
	`equip_id` bigint(30) NOT NULL,\
	PRIMARY KEY (`id`) USING BTREE\
	) ENGINE=InnoDB AUTO_INCREMENT=37 DEFAULT CHARSET=utf8 ROW_FORMAT=COMPACT;"


#define E_CREATETB_EQUIP_ASSET_TMPL "CREATE TABLE IF NOT EXISTS   `beopdata`.`equip_asset_tmpl` (\
`id` bigint(30) NOT NULL AUTO_INCREMENT,\
	`tmpl_def_id` bigint(30) NOT NULL,\
	`cn_name` varchar(255) NOT NULL,\
	`en_name` varchar(255) NOT NULL,\
	`ui_type` tinyint(1) NOT NULL,\
	`group_num` tinyint(1) NOT NULL DEFAULT '0',\
	`sort_num` int(11) NOT NULL,\
	`project_id` int(11) NOT NULL,\
	PRIMARY KEY (`id`)\
	) ENGINE=InnoDB AUTO_INCREMENT=364 DEFAULT CHARSET=utf8;"

#define E_CREATETB_EQUIP_ASSET_TMPL_DEF "CREATE TABLE IF NOT EXISTS   `beopdata`.`equip_asset_tmpl_def` (\
`id` bigint(30) NOT NULL AUTO_INCREMENT,\
	`name` varchar(30) NOT NULL,\
	`project_id` bigint(30) NOT NULL,\
	`describe` varchar(255) DEFAULT NULL,\
	PRIMARY KEY (`id`)\
	) ENGINE=InnoDB AUTO_INCREMENT=62 DEFAULT CHARSET=utf8;"

#define E_CREATETB_EQUIP_LIST "CREATE TABLE IF NOT EXISTS   `beopdata`.`equip_list` (\
`id` int(10) unsigned NOT NULL AUTO_INCREMENT,\
	`default_equipType` varchar(30) DEFAULT NULL,\
	`projId` varchar(11) DEFAULT NULL,\
	`name` varchar(50) DEFAULT NULL,\
	`description` varchar(200) DEFAULT NULL,\
	`online_addr` varchar(100) DEFAULT NULL,\
	`maintenanceStatus` int(11) DEFAULT NULL,\
	`repairStatus` int(11) DEFAULT NULL,\
	`repairResponsiblePerson` varchar(15) DEFAULT NULL,\
	`installLocation` varchar(50) DEFAULT NULL,\
	`communicateStatus` int(11) DEFAULT NULL,\
	`warningStatus` int(11) DEFAULT NULL,\
	`area_id` varchar(50) DEFAULT NULL,\
	`qrcode` varchar(200) DEFAULT NULL,\
	`model_id` int(11) DEFAULT NULL,\
	`system_id` varchar(11) DEFAULT NULL,\
	PRIMARY KEY (`id`) USING BTREE\
	) ENGINE=InnoDB AUTO_INCREMENT=832 DEFAULT CHARSET=utf8;"

#define E_CREATETB_EQUIP_PARAM "CREATE TABLE IF NOT EXISTS   `beopdata`.`equip_param` (\
`id` int(11) NOT NULL AUTO_INCREMENT,\
	`equip_id` int(11) NOT NULL,\
	`paramCode` varchar(200) NOT NULL,\
	`minValue` int(10) NOT NULL,\
	`maxValue` int(10) NOT NULL,\
	`paramUnit` varchar(200) NOT NULL,\
	`paramName` varchar(100) DEFAULT NULL,\
	`tmp` varchar(100) DEFAULT NULL,\
	`paramCommand` varchar(1024) DEFAULT NULL,\
	PRIMARY KEY (`id`) USING BTREE\
	) ENGINE=InnoDB AUTO_INCREMENT=4092 DEFAULT CHARSET=utf8;"

#define E_CREATETB_EQUIP_PARAM_TMPL "CREATE TABLE  IF NOT EXISTS  `beopdata`.`equip_param_tmpl` (\
`id` int(11) NOT NULL AUTO_INCREMENT,\
	`tmpl_def_id` int(11) NOT NULL,\
	`paramName` varchar(200) NOT NULL,\
	`paramCode` varchar(200) NOT NULL,\
	`minValue` int(10) DEFAULT NULL,\
	`maxValue` int(10) DEFAULT NULL,\
	`paramUnit` varchar(10) DEFAULT NULL,\
	`sort_num` int(11) NOT NULL DEFAULT '0',\
	PRIMARY KEY (`id`) USING BTREE\
	) ENGINE=InnoDB AUTO_INCREMENT=249 DEFAULT CHARSET=utf8;"


#define E_CREATETB_system_list "CREATE TABLE  `beopdata`.`system_list` (\
	`id` int(10) unsigned NOT NULL AUTO_INCREMENT,\
	`system_name` varchar(30) DEFAULT NULL,\
	`system_desc` varchar(11) DEFAULT NULL,\
	`createTime` datetime DEFAULT NULL,\
	`system_img` varchar(200) DEFAULT NULL,\
	`projId` varchar(100) DEFAULT NULL,\
	PRIMARY KEY (`id`) USING BTREE\
	) ENGINE=InnoDB AUTO_INCREMENT=53 DEFAULT CHARSET=utf8;"


#define E_CREATETB_inspact_area "CREATE TABLE  `beopdata`.`inspact_area` (\
	`id` int(11) NOT NULL AUTO_INCREMENT,\
	`areaName` varchar(200) DEFAULT NULL,\
	`description` varchar(200) DEFAULT NULL,\
	`seqno` int(10) DEFAULT NULL,\
	`projId` varchar(200) DEFAULT NULL,\
	`status` varchar(10) DEFAULT NULL,\
	`img` varchar(100) DEFAULT NULL,\
	PRIMARY KEY (`id`) USING BTREE\
	) ENGINE=InnoDB AUTO_INCREMENT=41 DEFAULT CHARSET=utf8 ROW_FORMAT=DYNAMIC;"


#define E_CREATETB_equip_operation "CREATE TABLE  `beopdata`.`equip_operation` (\
	`id` int(10) unsigned NOT NULL AUTO_INCREMENT,\
	`describe` varchar(200) NOT NULL DEFAULT '',\
	`operate_time` timestamp NOT NULL DEFAULT '0000-00-00 00:00:00',\
	`responsible_name` varchar(50) NOT NULL DEFAULT '',\
	`project_id` varchar(11) NOT NULL DEFAULT '',\
	PRIMARY KEY (`id`)\
	) ENGINE=InnoDB DEFAULT CHARSET=utf8;"


#define E_CREATETB_care_user "CREATE TABLE  `beopdata`.`care_user` (\
	`id` int(10) unsigned NOT NULL AUTO_INCREMENT,\
	`care_user` varchar(255) DEFAULT NULL,\
	`care_id` varchar(255) DEFAULT NULL,\
	`userid` varchar(255) DEFAULT NULL,\
	`username` varchar(255) DEFAULT NULL,\
	PRIMARY KEY (`id`)\
	) ENGINE=InnoDB DEFAULT CHARSET=utf8;"


#define E_CREATETB_care_history "CREATE TABLE  `beopdata`.`care_history` (\
	`title` varchar(255) DEFAULT NULL,\
	`operation_instruction` varchar(255) DEFAULT NULL,\
	`description` varchar(255) DEFAULT NULL,\
	`status` int(10) unsigned DEFAULT NULL,\
	`attention` varchar(255) DEFAULT NULL,\
	`createTime` timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP,\
	`id` int(10) unsigned NOT NULL DEFAULT '0',\
	`deviceName` varchar(255) DEFAULT NULL,\
	`selTime` timestamp NOT NULL DEFAULT '0000-00-00 00:00:00',\
	`checkContent` varchar(255) DEFAULT NULL,\
	`projId` varchar(11) NOT NULL DEFAULT '',\
	PRIMARY KEY (`id`)\
	) ENGINE=InnoDB DEFAULT CHARSET=utf8;"




#define E_CREATETB_REALTIMEDATA_INPUT_DLT_645	"CREATE TABLE IF NOT EXISTS beopdata.`realtimedata_input_dlt645` (\
`time` timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,\
`pointname` varchar(255) NOT NULL DEFAULT '',\
`pointvalue` varchar(2560) NOT NULL DEFAULT '',\
PRIMARY KEY (`pointname`)\
) ENGINE=MEMORY DEFAULT CHARSET=utf8;"

#define E_CREATETB_REALTIMEDATA_OUTPUT_DLT_645 "CREATE TABLE IF NOT EXISTS beopdata.`realtimedata_output_dlt645` (\
`pointname` varchar(255) NOT NULL DEFAULT '',\
`pointvalue` varchar(2560) NOT NULL DEFAULT '',\
PRIMARY KEY (`pointname`)\
) ENGINE=MEMORY DEFAULT CHARSET=utf8;"




#define E_CREATETB_REALTIMEDATA_INPUT_ABSLC	"CREATE TABLE IF NOT EXISTS beopdata.`realtimedata_input_abslc` (\
`time` timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,\
`pointname` varchar(255) NOT NULL DEFAULT '',\
`pointvalue` varchar(2560) NOT NULL DEFAULT '',\
PRIMARY KEY (`pointname`)\
) ENGINE=MEMORY DEFAULT CHARSET=utf8;"

#define E_CREATETB_REALTIMEDATA_OUTPUT_ABSLC "CREATE TABLE IF NOT EXISTS beopdata.`realtimedata_output_abslc` (\
`pointname` varchar(255) NOT NULL DEFAULT '',\
`pointvalue` varchar(2560) NOT NULL DEFAULT '',\
PRIMARY KEY (`pointname`)\
) ENGINE=MEMORY DEFAULT CHARSET=utf8;"


