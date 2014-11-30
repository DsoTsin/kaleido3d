create database GameXDB;
create table GameXDB.Users (
	username char(64),
    passwd	 char(64),
    email	 char(128),
    sex		 bool);