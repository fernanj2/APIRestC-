#include <iostream>
#include <vector>
#include <string>
#include <algorithm>
#include <optional>
#include <map>
#include <nlohmann/json.hpp>
#include <cpprest/http_listener.h>
#include <cpprest/json.h>

using namespace std;
using namespace web;
using namespace web::http;
using namespace web::http::experimental::listener;
using json = nlohmann::json;

map<int, json> books = {
    {1, {"id": 1, "title": "La hojarasca", "description": "Good one", "author": "Gabo"}},
    {2, {"id": 2, "title": "El coronel no tiene quien le escriba", "description": "Interesting", "author": "Gabo"}}
};

void handle_get_books(http_request request)
{
    json response;
    response["books"] = books | map_values;
    request.reply(status_codes::OK, response);
}

void handle_get_book(http_request request)
{
    int book_id = stoi(request.relative_uri().path().substr(7));
    if (books.find(book_id) == books.end()) {
        request.reply(status_codes::NotFound);
        return;
    }

    json response;
    response["book"] = books[book_id];
    request.reply(status_codes::OK, response);
}

void handle_create_book(http_request request)
{
    if (!request.has_body()) {
        request.reply(status_codes::BadRequest);
        return;
    }

    auto body = request.extract_json().get();
    if (!body.is_object() || !body.has_field("title")) {
        request.reply(status_codes::BadRequest);
        return;
    }

    int id = books.rbegin()->first + 1;
    json book = {
        {"id", id},
        {"title", body["title"].get<string>()},
        {"description", body.value("description", "")},
        {"author", body.value("author", "")}
    };
    books[id] = book;

    json response;
    response["book"] = book;
    request.reply(status_codes::Created, response);
}

void handle_update_book(http_request request)
{
    int book_id = stoi(request.relative_uri().path().substr(7));
    if (books.find(book_id) == books.end()) {
        request.reply(status_codes::NotFound);
        return;
    }

    if (!request.has_body()) {
        request.reply(status_codes::BadRequest);
        return;
    }

    auto body = request.extract_json().get();
    if (!body.is_object()) {
        request.reply(status_codes::BadRequest);
        return;
    }

    json& book = books[book_id];
    if (body.has_field("title")) {
        book["title"] = body["title"].get<string>();
    }
    if (body.has_field("description")) {
        book["description"] = body["description"].get<string>();
    }
    if (body.has_field("author")) {
        book["author"] = body["author"].get<string>();
    }

    json response;
    response["book"] = book;
    request.reply(status_codes::OK, response);
}

void handle_delete_book(http_request request)
{
    int book_id = stoi(request.relative_uri().path().substr(7));
    if (books.find(book_id) == books.end()) {
        request.reply(status_codes::NotFound);
        return;
    }

    books.erase(book_id);

    json response;
    response["result"] = true;
    request.reply(status_codes::OK, response);
}

int main()
{
    http_listener listener("http://localhost:5000");

    listener.support(methods::GET, "/books", handle_get_books
