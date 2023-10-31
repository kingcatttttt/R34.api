#include <iostream>
#include <string>
#include <sstream>
#include <curl/curl.h>
#include <nlohmann/json.hpp>
using namespace std;

size_t WriteCallback(void* contents, size_t size, size_t nmemb, std::string* buffer) {
    size_t real_size = size * nmemb;
    buffer->append((char*)contents, real_size);
    return real_size;
}

int main() {
    CURL* curl;
    CURLcode res;
    string tag;
    cin >> tag;
    std::string api_url = "https://api.rule34.xxx/index.php?page=dapi&s=post&q=index&json=1&limit=1000&tags="+ tag;
    std::string response;

    curl_global_init(CURL_GLOBAL_DEFAULT);
    curl = curl_easy_init();

    if (curl) {
        curl_easy_setopt(curl, CURLOPT_URL, api_url.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);

        res = curl_easy_perform(curl);

        curl_easy_cleanup(curl);
    } else {
        std::cerr << "Failed to initialize CURL." << std::endl;
        return 1;
    }

    if (res != CURLE_OK) {
        std::cerr << "Failed to perform CURL request: " << curl_easy_strerror(res) << std::endl;
        return 1;
    }

    // Парсим ответ как JSON
    nlohmann::json jsonResponse = nlohmann::json::parse(response);

    // Проверяем, что ответ является массивом JSON
    if (jsonResponse.is_array()) {
        std::cout << "Total posts: " << jsonResponse.size() << std::endl;

        // Скачиваем изображения
        for (const auto& post : jsonResponse) {
            std::string imageUrl = post["file_url"];
            std::stringstream ss;
            ss << "image_" << post["id"] << ".jpg";
            std::string file_name = ss.str();
            
            CURL* img_curl;
            FILE* img_file;

            img_file = fopen(file_name.c_str(), "wb");
            if (img_file) {
                img_curl = curl_easy_init();
                if (img_curl) {
                    curl_easy_setopt(img_curl, CURLOPT_URL, imageUrl.c_str());
                    curl_easy_setopt(img_curl, CURLOPT_WRITEDATA, img_file);
                    curl_easy_perform(img_curl);
                    curl_easy_cleanup(img_curl);
                }
                fclose(img_file);
            } else {
                std::cerr << "Failed to open file for writing: " << file_name << std::endl;
            }
        }
    } else {
        std::cerr << "Invalid JSON response format." << std::endl;
    }

    curl_global_cleanup();
    return 0;
}
