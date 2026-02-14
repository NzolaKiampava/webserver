/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: nkiampav <nkiampav@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/08 19:57:13 by nkiampav          #+#    #+#             */
/*   Updated: 2026/02/14 15:07:32 by nkiampav         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CLIENT_HPP
#define CLIENT_HPP

#include "Request.hpp"
#include "Response.hpp"
#include <string>
#include <ctime>

class Client {
private:
    int         _socket;
    std::string _ip_address;
    std::string _buffer;            // Buffer para dados recebidos (Request)
    std::string _response_buffer;   // Buffer para dados a enviar (Response)
    
    bool        _request_complete;
    bool        _response_ready;
    bool        _response_sent;
    size_t      _bytes_sent;
    
    // Controle de tempo para Timeout
    time_t      _last_activity;

    // Objetos de processamento (Membro 2)
    Request     _request;
    Response    _response;

    void        _check_request_complete();

public:
    Client(int socket, const std::string& ip);
    ~Client();

    // Getters
    int             get_socket() const;
    std::string     get_ip() const;
    std::string&    get_buffer();
    bool            is_request_complete() const;
    bool            has_response_to_send() const;
    bool            response_sent() const;
    time_t          get_last_activity() const;

    // Setters e Processamento
    void            set_response_buffer(const std::string& response);
    void            update_activity();
    void            reset_for_next_request();

    // I/O
    void            receive_data();
    void            send_response();

    // Membro 2 getters
    Request&        get_request();
    Response&       get_response();
};

#endif