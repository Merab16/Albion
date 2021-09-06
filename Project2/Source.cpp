#include <Windows.h>
#include <winhttp.h>
#include <iostream>
#include <string>
#include <chrono>

// Программа работает с API игры Albion Online (https://www.albion-online-data.com/)
// Сделана для отслеживания цен предметов в игровом мире, была заброшена так как данные о ценах предметов обновляются
// по разному и невозможно точно оценивать ситуацию



#pragma comment(lib, "Winhttp.lib")

std::string const DATA = "08-11"; // Нужная дата

class Item {
public:
	explicit Item(std::string _item_id, std::string _quality, std::string _price, std::string _count) :
		item_id(_item_id), quality(_quality), price(_price), count(_count) {
		//list_count++;
	}

	Item() {
		item_id = "ID";
		quality = "0";
		price = "0";
		count = "0";
	}

	void setInfo(std::string _item_id, std::string _quality, std::string _count, std::string _price, std::string _data) {
		this->item_id = _item_id;
		this->quality = _quality;
		this->price = _price;
		this->count = _count;
		this->data = _data;
		list_count++;
	}

	int& getListCount() {
		return list_count;
	}

	std::string& getItemId() {
		return this->item_id;
	}
	std::string& getQuality() {
		return this->quality;
	}
	std::string& getPrice() {
		return this->price;
	}
	std::string& getCount() {
		return this->count;
	}
	std::string& getData() {
		return this->data;
	}

private:
	std::string item_id;
	std::string quality;
	std::string price;
	std::string count;
	std::string data;
	static int list_count;

};

int Item::list_count = 0;

std::wstring s2ws(const std::string& s)
{
	int len;
	int slength = (int)s.length() + 1;
	len = MultiByteToWideChar(CP_ACP, 0, s.c_str(), slength, 0, 0);
	wchar_t* buf = new wchar_t[len];
	MultiByteToWideChar(CP_ACP, 0, s.c_str(), slength, buf, len);
	std::wstring r(buf);
	delete[] buf;
	return r;
}

std::string getData() {

	std::string _bref = "/api/v2/stats/prices/";
	std::string _eref = "T4_BAG?locations=&qualities=";
	std::string _ref = _bref + _eref;

	std::wstring stemp = s2ws(_ref);
	LPCWSTR result = stemp.c_str();




	HINTERNET hSession = WinHttpOpen(L"HTTP Example/1.0", WINHTTP_ACCESS_TYPE_DEFAULT_PROXY, WINHTTP_NO_PROXY_NAME, WINHTTP_NO_PROXY_BYPASS, 0);

	HINTERNET hConnect = WinHttpConnect(hSession, L"www.albion-online-data.com", INTERNET_DEFAULT_HTTPS_PORT, 0);


	//HINTERNET hRequest = WinHttpOpenRequest(hConnect, L"GET", L"api.vk.com/method/getLists?&access_token=TOKEN&v=V", NULL, WINHTTP_NO_REFERER, WINHTTP_DEFAULT_ACCEPT_TYPES, WINHTTP_FLAG_SECURE);
	HINTERNET hRequest = WinHttpOpenRequest(hConnect, L"GET", result, NULL, WINHTTP_NO_REFERER, WINHTTP_DEFAULT_ACCEPT_TYPES, WINHTTP_FLAG_SECURE);
	WinHttpSendRequest(hRequest, WINHTTP_NO_ADDITIONAL_HEADERS, 0, WINHTTP_NO_REQUEST_DATA, 0, 0, 0);

	WinHttpReceiveResponse(hRequest, NULL);



	DWORD dwSize{}, dwDownloaded{};
	char* pszOutBuffer{};
	std::string buffer{};
	std::string value{};
	do {
		dwSize = 0;
		if (!WinHttpQueryDataAvailable(hRequest, &dwSize))
			std::cout << "Error " << GetLastError() << "in WinHttpQueryDataAvailable.\n";
		pszOutBuffer = new char[dwSize + 1];
		if (!pszOutBuffer) {
			std::cout << "Out of memory\n";
			dwSize = 0;
		}
		else {
			ZeroMemory(pszOutBuffer, dwSize + 1);
			if (!WinHttpReadData(hRequest, (LPVOID)pszOutBuffer, dwSize, &dwDownloaded))
				std::cout << "Error " << GetLastError() << "in WinHttpReadData.\n";
			else {
				//std::cout << pszOutBuffer << std::endl;
			}
			buffer += pszOutBuffer;
			delete[] pszOutBuffer;


		}

	} while (dwSize > 0);

	return buffer;


}

std::string filterData(std::string item_id, std::string city) {
	std::string buffer = getData();
	std::string _str;
	std::string str;
	bool flag = false;
	bool qflag = false;

	for (size_t i = 0; i < buffer.length(); i++) {
		if (buffer[i] == city[0]
			&& buffer[i + 1] == city[1]
			&& buffer[i + 2] == city[2]
			&& buffer[i + 3] == city[3]) {
			//std::cout << "Find\n";

			for (size_t k = i; buffer[k] != '}'; k++) {
				if (!qflag
					&& buffer[k] == 'q'
					&& buffer[k + 1] == 'u'
					) {
					qflag = true;
					for (int l = k; buffer[l - 1] != '}'; l++)
						_str += buffer[l];
				}

			}
			str += _str;
			//std::cout << _str << std::endl;
			_str.clear();
			flag = false;
			qflag = false;

		}

	}



	return str;
}

void parsData(std::string buffer, std::string item_id, Item list[], bool flag) {
	bool isFull = false;
	bool first = true;
	bool qualIsChanched = false;
	std::string qual;
	std::string price;
	std::string count;
	std::string date;
	if (!flag)
		for (size_t i = 0; i < buffer.length(); i++) {
			if (buffer[i] == 'q'
				&& buffer[i + 1] == 'u') {

				qual = "";
				qual += buffer[i + 9];
			}
			else if (buffer[i] == 'c'
				&& buffer[i + 1] == 'o') {
				for (size_t k = i + 7; buffer[k] != ','; k++)
					count += buffer[k];
			}
			else if (buffer[i] == 'p'
				&& buffer[i + 1] == 'r') {
				for (size_t k = i + 7; buffer[k] != ','; k++)
					price += buffer[k];
				isFull = true;
			}
			if (isFull) {
				//std::cout << list->getListCount() << std::endl;
				//std::cout << qual << std::endl;
				list[list->getListCount()].setInfo(item_id, qual, count, price, DATA);
				count = "";
				price = "";
				isFull = false;
			}
		}


	if (flag) {
		for (size_t i = 0; i < buffer.length(); i++) {
			if (buffer[i] == 'q'
				&& buffer[i + 1] == 'u') {

				qual = buffer[i + 9];
				qualIsChanched = true;
			}
			else if (qualIsChanched
				&& buffer[i] == 'p'
				&& buffer[i + 1] == 'r') {
				if (first) {
					for (size_t k = i + 11; buffer[k] != ','; k++)
						price += buffer[k];
					first = false;
				}
				else {
					for (size_t k = i + 17; buffer[k] != '"'; k++)
						date += buffer[k];
					isFull = true;
					qualIsChanched = false;
				}
			}

			if (isFull) {
				//std::cout << list->getListCount() << std::endl;
				//std::cout << qual << std::endl;
				list[list->getListCount()].setInfo(item_id, qual, "1", price, date);
				price = "";
				date = "";
				first = true;
				isFull = false;

			}
		}
	}


} 

void displayList(Item list[], std::string city) {
	std::string citys[5] = { "Martlock", "FortSterling" , "Thetford" ,"Lymhurst","Bridgewatch" };
	int counter = 0;
	std::cout << "City: " << city << std::endl;
	std::cout << "Number\t\tItemID\t\tQuality\t\tCount\t\tPrice\t\tData\n";
	for (int i = 0; i < list->getListCount(); i++) {
		if (i % 5 == 0) {
			std::cout << citys[counter] << std::endl;
			counter++;
			//std::cout << std::endl;
		}
		std::cout << (i + 1);
		std::cout << ".\t";
		std::cout << list[i].getItemId() + "\t";
		std::cout << list[i].getQuality() + "\t\t";
		std::cout << list[i].getCount() + "\t\t";
		std::cout << list[i].getPrice() + "\t\t";
		std::cout << list[i].getData() << std::endl;
	}
}

int main() {

	auto start = std::chrono::high_resolution_clock::now();
	std::chrono::duration<float> duration;

	std::cout << getData() << std::endl;


	Item* item_list = new Item[1000];
	std::string const item_id = "T4_BAG";
	std::string const city = "Martlock";

	std::string str = filterData(item_id, "Martlock");
	parsData(str, item_id, item_list, true);
	std::string str1 = filterData(item_id, "FortSterling");
	parsData(str1, item_id, item_list, true);
	std::string str2 = filterData(item_id, "Thetford");
	parsData(str2, item_id, item_list, true);
	std::string str3 = filterData(item_id, "Lymhurst");
	parsData(str3, item_id, item_list, true);
	std::string str4 = filterData(item_id, "Bridgewatch");
	parsData(str4, item_id, item_list, true);




	displayList(item_list, "ALL");

	auto end = std::chrono::high_resolution_clock::now();
	duration = end - start;
	std::cout << "\nExecuting time: " << duration.count() * 1.f << "s" << std::endl;


	delete[] item_list;
	return 0;
}