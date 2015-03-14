create database GameXDB;

/* 用户数据表 */
create table GameXDB.Users (
	username 	char(64) unique,
    passwd	 	char(64),
    email	 	char(128),
    sex		 	bool);
    
/* 登陆统计 */
create table GameXDB.Statistic (
	username	char(64),
	mac			char(64) unique,
    count		long,
    lastime		long
);