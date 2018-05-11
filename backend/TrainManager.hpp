#pragma once

#include <iostream>
#include <fstream>
#include <functional>
#include "exceptions.h"
#include "bptree.hpp"
#include "vector.hpp"
#include "map.hpp"
#include "pair.hpp"
#include "String.hpp"
#include "date.hpp"

class TrainManager {
private:
	struct Train{
		TrainID id;
		TrainName name;
		Catalog catalog;
		int station_num;
		int seat_num;
		long route_pos;                    // position of this train's route in route_file 
		long ticket_price_pos;             // position of this train's ticket_price in ticket_price_file
		long ticket_left_pos;              // position of this train's ticket_left in ticket_left_file
		Seat seat[15];
		int open;
		int sale;
		Train() : station_num(0), seat_num(0), open(0), sale(0) {}
	};

	struct Station {
		Location loc;
		Time arrive;
		Time depart;
		Time stop;
	};

	typedef bptree<TrainID, Train> TrainRecord;

	struct BindKey {
		Location loc;
		TrainID train_id;
		BindKey(const Location & _loc = Location(), const TrainID & _train_id = TrainID()) : loc(_loc), train_id(_train_id) {}
		bool operator==(const BindKey & other) {
			return (loc == other.loc && train_id == other.train_id);
		}
		bool operator<(const BindKey & other) const {
			if (loc < other.loc) return 1;
			if (loc > other.loc) return 0;
			return train_id < other.train_id;
		}
	};

	struct BindValue {
		int station_no;
		Catalog catalog;
		BindValue(int _no = -1, const Catalog & _catalog = Catalog()) : station_no(_no), catalog(_catalog) {}
	};

	typedef bptree<BindKey, BindValue> StationRecord;

	struct TicketKey {
		UserID user_id;
		Date date;
		Catalog catalog;
		TrainID train_id;
		Location from;
		Location to;
		TicketKey(const UserID & _user_id = UserID(), const Date & _date = Date(), Catalog _catalog = Catalog(), 
			const TrainID & _train_id = TrainID(), const Location & _from = Location(), const Location & _to = Location()) :
			user_id(_user_id), date(_date), catalog(_catalog), train_id(_train_id), from(_from), to(_to) {}
		bool operator==(const TicketKey & other) {
			return (user_id == other.user_id && date == other.date && catalog == other.catalog 
				&& train_id == other.train_id && from == other.from && to == other.to);
		}
		bool operator<(const TicketKey & other) const {
			if (user_id < other.user_id) return 1;
			if (user_id > other.user_id) return 0;
			if (date < other.date) return 1;
			if (date > other.date) return 0;
			if (catalog < other.catalog) return 1;
			if (catalog > other.catalog) return 0;
			if (train_id < other.train_id) return 1;
			if (train_id > other.train_id) return 0;
			if (from < other.from) return 1;
			if (from > other.from) return 0;
			return to < other.to;
		}
	};

	struct TicketValue {
		Time depart;
		Time arrive;
		Seat seat[15];
		double price[15];
		int num[15] = { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 };
		TicketValue(const Time & _depart = Time("xx:xx"), const Time & _arrive = Time("xx:xx")) :
			depart(_depart), arrive(_arrive) {}
	};

	typedef bptree<TicketKey, TicketValue> OrderRecord;

	TrainRecord train_record;
	StationRecord station_record;
	OrderRecord order_record;
	String<20> route_file;
	String<20> ticket_price_file;
	String<20> ticket_left_file;

	// for a specific train, query ticket.
	void query_ticket(const Train & train, const Date & date, int s1_no, int s2_no, std::istream & is = std::cin, std::ostream & os = std::cout) {
		
		std::fstream iofile;

		Station * s_array = new Station[train.station_num];          // s_array: station_array
		iofile.open(route_file.getAddress());
		iofile.seekg(train.route_pos, std::ios::beg);
		iofile.read(reinterpret_cast<char *> (s_array), sizeof(Station) * train.station_num);
		iofile.close();

		int tp_num = train.seat_num * train.station_num;             // tp_num: ticket_price_num 
		double * tp_array = new double[tp_num];                      // tp_array: ticket_price_array
		iofile.open(ticket_price_file.getAddress());
		iofile.seekg(train.ticket_price_pos, std::ios::beg);
		iofile.read(reinterpret_cast<char *> (tp_array), sizeof(double) * tp_num);
		iofile.close();

		int day_no = date - start_date;
		int tl_num = train.seat_num * train.station_num;                     // tl_num: ticket_left_num
		int * tl_array = new int[tl_num];                                    // tl_array: ticket_left_array
		iofile.open(ticket_left_file.getAddress());
		iofile.seekg(train.ticket_left_pos + day_no * tl_num * sizeof(int), std::ios::beg);
		iofile.read(reinterpret_cast<char *> (tl_array), sizeof(int) * tl_num);
		iofile.close();

		os << train.id << ' ' << s_array[s1_no].loc << ' ' << date << ' ' << s_array[s1_no].depart
			<< ' ' << s_array[s2_no].loc << ' ' << date << ' ' << s_array[s2_no].arrive << ' ';
		for (int i = 0; i < train.seat_num; ++i) {
			int ticket_left = 2000;
			double ticket_price = 0;
			for (int j = s1_no + 1; j <= s2_no; ++j) {
				if (tl_array[j + i * train.station_num] < ticket_left) {
					ticket_left = tl_array[j + i * train.station_num];
				}
				ticket_price += tp_array[j + i * train.station_num];
			}
			os << train.seat[i] << ' ' << ticket_left << ' ' << ticket_price << ' ';
		}
		os << '\n';

		delete[] s_array;
		delete[] tp_array;
		delete[] tl_array;
	}

public:
	TrainManager() : train_record("train_record", "index_train_record"), 
		station_record("station_record", "index_station_record"),
		order_record("order_record", "index_order_record"),
		route_file("route_record"), ticket_price_file("ticket_price_record"), ticket_left_file("ticket_left_record") {
		std::fstream iofile;
		iofile.open(route_file.getAddress());
		if (!iofile) {
			init();
		}
		else {
			iofile.close();
		}
	}

	void init() {
		train_record.init();
		station_record.init();
		order_record.init();
		std::ofstream out;
		out.open(route_file.getAddress());
		out.close();
		out.open(ticket_price_file.getAddress());
		out.close();
		out.open(ticket_left_file.getAddress());
		out.close();
	}
	
	int sale_train(const TrainID & train_id, std::istream & is = std::cin, std::ostream & os = std::cout) {
		Train train = train_record.find(train_id);
		if (train.station_num == 0) return 0;
		if (train.open == 1) return 0;
		train.open = 1;
		train_record.set(train_id, train);
		return 1;
	}

	int query_train(const TrainID & train_id, std::istream & is = std::cin, std::ostream & os = std::cout) {

		Train train = train_record.find(train_id);
		if (train.station_num == 0) return 0;
		//if (train.open == 0) return 0;
		os << train.id << ' ' << train.name << ' ' << train.catalog 
			<< ' ' << train.station_num << ' ' << train.seat_num << ' ';
		for (int i = 0; i < train.seat_num; ++i) {
			os << train.seat[i] << ' ';
		}
		os << '\n';
		
		std::fstream iofile;
		
		iofile.open(route_file.getAddress());
		iofile.seekg(train.route_pos, std::ios::beg);
		Station * s_array = new Station[train.station_num];          // s_array: station_array
		iofile.read(reinterpret_cast<char *> (s_array), sizeof(Station) * train.station_num);
		iofile.close();

		iofile.open(ticket_price_file.getAddress());
		iofile.seekg(train.ticket_price_pos, std::ios::beg);
		int tp_num = train.seat_num * train.station_num;           // tp_num: ticket_price_num 
		double * tp_array = new double[tp_num];                      // tp_array: ticket_price_array
		iofile.read(reinterpret_cast<char *> (tp_array), sizeof(double) * tp_num);
		iofile.close();

		for (int i = 0; i < train.station_num; ++i) {
			os << s_array[i].loc << ' ' << s_array[i].arrive << ' ' 
				<< s_array[i].depart << ' ' << s_array[i].stop << ' ';
			for (int j = 0; j < train.seat_num; ++j) {
				os << '$' << tp_array[i + j * train.station_num] << ' ';
				//os << '��' << tp_array[i + j * train.station_num] << ' ';
			}
			os << '\n';
		}

		delete[] s_array;
		delete[] tp_array;

		return 1;
	}
	
	int add_train(const TrainID & train_id, std::istream & is = std::cin, std::ostream & os = std::cout) {

		Train train;
		train.id = train_id;
		is >> train.name >> train.catalog >> train.station_num >> train.seat_num;
		//if (train_record.count(train.id) == 1) return 0;
		if ((train_record.find(train.id)).station_num != 0) {
			for (int i = 0; i < train.seat_num; ++i) {
				is >> train.seat[i];
			}
			Station station;
			double ticket_price;
			for (int i = 0; i < train.station_num; ++i) {
				is >> station.loc >> station.arrive >> station.depart >> station.stop;
				char ch;
				for (int j = 0; j < train.seat_num; ++j) {
					is >> ch >> ticket_price;
					//is >> ch >> ch >> ch >> ticket_price;
				}
			}
			return 0;
		}
		for (int i = 0; i < train.seat_num; ++i) {
			is >> train.seat[i];
		}

		Station * s_array = new Station[train.station_num];          // s_array: station_array
		int tp_num = train.seat_num * train.station_num;           // tp_num: ticket_price_num 
		double * tp_array = new double[tp_num];                      // tp_array: ticket_price_array
		
		for (int i = 0; i < train.station_num; ++i) {
			is >> s_array[i].loc >> s_array[i].arrive >> s_array[i].depart >> s_array[i].stop;
			char ch;
			for (int j = 0; j < train.seat_num; ++j) {
				is >> ch >> tp_array[i + j * train.station_num];
				//is >> ch >> ch >> ch >> tp_array[i + j * train.station_num];
			}
			BindKey bind_key(s_array[i].loc, train.id);
			BindValue bind_value(i, train.catalog);
			station_record.insert(bind_key, bind_value);
		}

		std::fstream iofile;
		
		iofile.open(route_file.getAddress());
		iofile.seekp(0, std::ios::end);
		train.route_pos = iofile.tellp();
		iofile.write(reinterpret_cast<char *> (s_array), sizeof(Station) * train.station_num);
		iofile.close();
		
		iofile.open(ticket_price_file.getAddress());
		iofile.seekp(0, std::ios::end);
		train.ticket_price_pos = iofile.tellp();
		iofile.write(reinterpret_cast<char *> (tp_array), sizeof(double) * tp_num);
		iofile.close();
		
		delete[] s_array;
		delete[] tp_array;
		
		int day_num = (end_date - start_date) + 1;
		int tl_num = day_num * train.seat_num * train.station_num;         // tl_num: ticket_left_num
		
		int * tl_array = new int[tl_num];                                    // tl_array: ticket_left_array
		for (int i = 0; i < tl_num; ++i) {
			tl_array[i] = 2000;
		}

		iofile.open(ticket_left_file.getAddress());
		iofile.seekp(0, std::ios::end);
		train.ticket_left_pos = iofile.tellp();
		iofile.write(reinterpret_cast<char *> (tl_array), sizeof(int) * tl_num);
		iofile.close();

		delete[] tl_array;
		
		train_record.insert(train.id, train);
		
		return 1;
	}
	
	int delete_train(TrainID & train_id, std::istream & is = std::cin, std::ostream & os = std::cout) {
		Train train = train_record.find(train_id);
		if (train.station_num = 0) return 0;
		if (train.sale == 1) return 0;
	
		std::fstream iofile;
		iofile.open(route_file.getAddress());
		iofile.seekg(train.route_pos, std::ios::beg);
		Station * s_array = new Station[train.station_num];           //s_array: station_array
		iofile.read(reinterpret_cast<char *> (s_array), sizeof(Station) * train.station_num);
		iofile.close();
		
		for (int i = 0; i < train.station_num; ++i) {
			BindKey bind_key(s_array[i].loc, train.id);
			station_record.remove(bind_key);
		}

		delete[] s_array;
		
		train_record.remove(train_id);
		
		return 1;
	}

	int modify_train(std::istream & is = std::cin, std::ostream & os = std::cout) {
		TrainID train_id;
		is >> train_id;
		int flag = delete_train(train_id);
		if (flag == 0) return 0;
		return add_train(train_id);
	}

	int query_ticket(std::istream & is = std::cin, std::ostream & os = std::cout) {
		Location loc1, loc2;
		Date date;
		CatalogList catalog_list;
		is >> loc1 >> loc2 >> date >> catalog_list;
		
		BindKey bind_key1(loc1), bind_key2(loc2);
		sjtu::vector<sjtu::pair<BindKey, BindValue> > p_array1, p_array2;       // p_array: array of train_id that pass the station
		auto same_loc = [](const BindKey & bk1, const BindKey & bk2)->bool { return bk1.loc < bk2.loc; };
		station_record.search(p_array1, bind_key1, same_loc);
		station_record.search(p_array2, bind_key2, same_loc);

		struct Choice {
			Train train;
			int from;     // number of start station
			int to;       // number of end station
			Choice(const Train & _train, int _from, int _to) : train(_train), from(_from), to(_to) {}
		};

		sjtu::vector<Choice> c_array;                                        // t_array: choice which may statisfy condition
		auto iter1 = p_array1.begin(), iter2 = p_array2.begin();
		while (iter1 != p_array1.end() && iter2 != p_array2.end()) {
			TrainID id1((*iter1).first.train_id), id2((*iter2).first.train_id);
			if (id1 < id2) {
				++iter1;
				continue;
			}
			if (id2 < id1) {
				++iter2;
				continue;
			}
			int s1_no = (*iter1).second.station_no, s2_no = (*iter2).second.station_no;
			char catalog = (*iter1).second.catalog;
			bool in_catalog_list = false;
			int i = 0;
			while (catalog_list[i] != '\0') {
				if (catalog == catalog_list[i]) {
					in_catalog_list = true;
					break;
				}
				++i;
			}
			if (in_catalog_list == true && s1_no < s2_no) {
				Train train = train_record.find(id1);
				if (train.station_num != 0 && train.open == 1) {
					Choice choice(train, s1_no, s2_no);
					c_array.push_back(choice);
				}
			}
			++iter1;
			++iter2;
		}

		os << c_array.size() << '\n';
		
		for (auto iter = c_array.begin(); iter != c_array.end(); ++iter) {
			query_ticket((*iter).train, date, (*iter).from, (*iter).to, is, os);
		}

		return 1;
	}

	int query_transfer(std::istream & is = std::cin, std::ostream & os = std::cout) {}

	int buy_ticket(const UserID & user_id, std::istream & is = std::cin, std::ostream & os = std::cout) {
		int num;
		TrainID train_id;
		Location loc1, loc2;
		Date date;
		Seat seat_kind;
		is >> num >> train_id >> loc1 >> loc2 >> date >> seat_kind;

		if (date < start_date || date > end_date) {
			return 0;
		}
		Train train = train_record.find(train_id);
		if (train.station_num == 0) return 0;
		if (train.open == 0) return 0;

		std::fstream iofile;

		Station * s_array = new Station[train.station_num];          // s_array: station_array
		iofile.open(route_file.getAddress());
		iofile.seekg(train.route_pos, std::ios::beg);
		iofile.read(reinterpret_cast<char *> (s_array), sizeof(Station) * train.station_num);
		iofile.close();

		int s1_no, s2_no;
		for (int i = 0; i < train.station_num; ++i) {
			if (s_array[i].loc == loc1) {
				s1_no = i;
				break;
			}
		}
		for (int i = 0; i < train.station_num; ++i) {
			if (s_array[i].loc == loc2) {
				s2_no = i;
				break;
			}
		}
		if (s1_no >= s2_no) {
			delete[] s_array;
			return 0;
		}

		int seat_no = -1;
		for (int i = 0; i < train.seat_num; ++i) {
			if (train.seat[i] == seat_kind) {
				seat_no = i;
				break;
			}
		}
		if (seat_no == -1) {
			delete[] s_array;
			return 0;
		}
		int day_no = date - start_date;
		int * tl_array = new int[train.station_num];                                    // tl_array: ticket_left_array
		iofile.open(ticket_left_file.getAddress());
		iofile.seekg(train.ticket_left_pos + (day_no * train.seat_num * train.station_num  
			+ seat_no * train.station_num) * sizeof(int), std::ios::beg);
		iofile.read(reinterpret_cast<char *> (tl_array), sizeof(int) * train.station_num);

		int ticket_left = 2000;
		for (int i = s1_no + 1; i <= s2_no; ++i) {
			if (tl_array[i] < ticket_left) {
				ticket_left = tl_array[i];
			}
		}
		if (ticket_left < num) {
			iofile.close();
			delete[] s_array;
			delete[] tl_array;
			return 0;
		}
		for (int i = s1_no + 1; i <= s2_no; ++i) {
			tl_array[i] -= num;
		}
		
		iofile.seekp(train.ticket_left_pos + (day_no * train.seat_num * train.station_num 
			+ seat_no * train.station_num) * sizeof(int), std::ios::beg);
		iofile.write(reinterpret_cast<char *> (tl_array), sizeof(int) * train.station_num);
		iofile.close();

		if (train.sale == 0) {
			train.sale = 1;
		}
		train_record.set(train_id, train);

		TicketKey ticket_key(user_id, date, train.catalog, train.id, loc1, loc2);
		TicketValue ticket_value = order_record.find(ticket_key);
		if (ticket_value.depart == Time("xx:xx")) {
			ticket_value.depart = s_array[s1_no].depart;
			ticket_value.arrive = s_array[s2_no].arrive;
			for (int i = 0; i < train.seat_num; ++i) {
				ticket_value.seat[i] = train.seat[i];
			}

			iofile.open(ticket_price_file.getAddress());
			iofile.seekg(train.ticket_price_pos, std::ios::beg);
			int tp_num = train.seat_num * train.station_num;             // tp_num: ticket_price_num 
			double * tp_array = new double[tp_num];                      // tp_array: ticket_price_array
			iofile.read(reinterpret_cast<char *> (tp_array), sizeof(double) * tp_num);
			iofile.close();
			
			for (int i = 0; i < train.seat_num; ++i) {
				ticket_value.price[i] = 0;
				ticket_value.num[i] = 0;
				for (int j = s1_no + 1; j <= s2_no; ++j) {
					ticket_value.price[i] += tp_array[j + i * train.station_num];
				}
			}
			delete[] tp_array;
			ticket_value.num[seat_no] += num;

			order_record.insert(ticket_key, ticket_value);
		}
		else {
			ticket_value.num[seat_no] += num;
			order_record.set(ticket_key, ticket_value);
		}

		delete[] tl_array;
		delete[] s_array;

		return 1;
	}

	int query_order(const UserID & user_id, std::istream & is = std::cin, std::ostream & os = std::cout) {
		Date date;
		CatalogList catalog_list;
		is >> date >> catalog_list;
		if (date < start_date || date > end_date) {
			return -1;
		}
		sjtu::vector<sjtu::vector<sjtu::pair<TicketKey, TicketValue> > > t_array;     // t_array: ticket_array
		int i = 0;
		while (catalog_list[i] != '\0') {
			TicketKey ticket_key(user_id, date, catalog_list[i]);
			sjtu::vector<sjtu::pair<TicketKey, TicketValue> > tc_array;               // tc_array: certain_catalog_ticket_array
			auto meet_condition = [](const TicketKey & tk1, const TicketKey & tk2)->bool { 
				if (tk1.user_id < tk2.user_id) return 1;
				if (tk1.user_id > tk2.user_id) return 0;
				if (tk1.date < tk2.date) return 1;
				if (tk1.date > tk2.date) return 0;
				return tk1.catalog < tk2.catalog;
			};
			order_record.search(tc_array, ticket_key, meet_condition);
			if (!tc_array.empty()) {
				t_array.push_back(tc_array);
			}
			++i;
		}
		int list_num = 0;
		for (int i = 0; i < t_array.size(); ++i) {
			list_num += t_array[i].size();
		}
		os << list_num << '\n';
		for (int i = 0; i < t_array.size(); ++i) {
			for (int j = 0; j < t_array[i].size(); ++j) {
				os << t_array[i][j].first.train_id << ' '
					<< t_array[i][j].first.from << ' ' << date << ' ' << t_array[i][j].second.depart << ' '
					<< t_array[i][j].first.to << ' ' << date << ' ' << t_array[i][j].second.arrive << ' ';
				int k = 0;
				while (t_array[i][j].second.num[k] != -1) {
					os << t_array[i][j].second.seat[k] << ' ' 
						<< t_array[i][j].second.num[k] << ' ' 
						<< t_array[i][j].second.price[k] << ' ';
					++k;
				}
				os << '\n';
			}
		}
		return 1;
	}

	int refund_ticket(const UserID & user_id, std::istream & is = std::cin, std::ostream & os = std::cout) {
		int num;
		TrainID train_id;
		Location loc1, loc2;
		Date date;
		Seat seat_kind;
		is >> num >> train_id >> loc1 >> loc2 >> date >> seat_kind;

		if (date < start_date || date > end_date) {
			return 0;
		}
		Train train = train_record.find(train_id);
		if (train.station_num == 0) return 0;
	
		int seat_no;
		for (int i = 0; i < train.seat_num; ++i) {
			if (seat_kind == train.seat[i]) {
				seat_no = i;
				break;
			}
		}

		TicketKey ticket_key(user_id, date, train.catalog, train.id, loc1, loc2);
		TicketValue ticket_value = order_record.find(ticket_key);
		if (ticket_value.num[seat_no] < num) {
			return 0;
		}
		ticket_value.num[seat_no] -= num;
		bool flag = false;
		for (int i = 0; i < train.seat_num; ++i) {
			if (ticket_value.num[i] > 0) {
				flag = true;
				break;
			}
		}
		if (flag == true) {
			order_record.set(ticket_key, ticket_value);
		}
		else {
			order_record.remove(ticket_key);
		}

		int s1_no, s2_no;
		std::fstream iofile;
		Station * s_array = new Station[train.station_num];          // s_array: station_array
		iofile.open(route_file.getAddress());
		iofile.seekg(train.route_pos, std::ios::beg);
		iofile.read(reinterpret_cast<char *> (s_array), sizeof(Station) * train.station_num);
		iofile.close();
		for (int i = 0; i < train.station_num; ++i) {
			if (s_array[i].loc == loc1) {
				s1_no = i;
				break;
			}
		}
		for (int i = 0; i < train.station_num; ++i) {
			if (s_array[i].loc == loc2) {
				s2_no = i;
				break;
			}
		}
		delete[] s_array;

		int day_no = date - start_date;
		int * tl_array = new int[train.station_num];                                    // tl_array: ticket_left_array
		iofile.open(ticket_left_file.getAddress());
		iofile.seekg(train.ticket_left_pos + (day_no * train.seat_num * train.station_num 
			+ seat_no * train.station_num) * sizeof(int), std::ios::beg);
		iofile.read(reinterpret_cast<char *> (tl_array), sizeof(int) * train.station_num);
		for (int i = s1_no + 1; i <= s2_no; ++i) {
			tl_array[i] += num;
		}
		iofile.seekp(train.ticket_left_pos + (day_no * train.seat_num * train.station_num 
			+ seat_no * train.station_num) * sizeof(int), std::ios::beg);
		iofile.write(reinterpret_cast<char *> (tl_array), sizeof(int) * train.station_num);
		iofile.close();
		delete[] tl_array;

		return 1;
	}

	
	void list_station(std::istream & is = std::cin, std::ostream & os = std::cout) {
		std::fstream iofile;
		iofile.open(route_file.getAddress());
		iofile.seekg(0, std::ios::end);
		long route_file_end = iofile.tellg();
		iofile.seekg(0, std::ios::beg);
		sjtu::map<Location, int> station_map;
		Station station;
		for (int i = 0; i < route_file_end; i += sizeof(Station)) {
			iofile.read(reinterpret_cast<char *> (&station), sizeof(Station));
			station_map[station.loc] = 1;
		}
		os << station_map.size() << '\n';
		for (auto iter = station_map.begin(); iter != station_map.end(); ++iter) {
			os << (*iter).first << ' ';
		}
		if (!station_map.empty()) {
			os << '\n';
		}
		iofile.close();
	}
	
};


