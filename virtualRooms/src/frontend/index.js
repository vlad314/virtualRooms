let rooms = document.getElementsByClassName("room");
let roomsLength = 1;
let createRoom = document.querySelector("#create");

let newRoomId = 0;

ws = new WebSocket('ws://localhost:9999');
ws.onopen = function (ev) {
    ws.send("reqid");
};

ws.onmessage = function (ev) {
    console.log("---- EV -----");
    let data = ev.data;
    let parsedData = parse_data(data);
    console.log("----- parsedData = ");
    console.log(parsedData);
    let firstw = parsedData[parsedData.length - 1];
    if (firstw == "rooms") {
        renderAll(parsedData);
        roomsLength = rooms.length;
    } else if (firstw == "joinchat") {
        let msg = parse_msg(data);
        renderChatBox(msg[1]);
    } else if (firstw == "chat") {
        let msg = parse_msg(data);
        let outp = "[";
        outp = outp.concat(msg[0]);
        outp = outp.concat("] : ");
        outp = outp.concat(msg[1]);
        outp = outp.concat("\n");
        messagesid.innerText += outp;
    }
    joinRoomFct();
}

function parse_users(s) {
    let semicol = s.search(';');
    var users_arr = [];
    let i = 0;
    let b = 0;
    let e = semicol;
    while (e != -1) {
        users_arr[i] = s.substring(b, e);
        s = s.substring(e + 1);
        e = s.search(';');
        i++;
    }
    users_arr[i] = s;
    return users_arr;
}

function parse_rooms(s) {
    let sOut = new Object();
    var rooms = [];
    let braceOpen = s.search('{');
    let roomII = s.substring(0, braceOpen);
    rooms[0] = roomII;
    let braceClosed = s.search('}');
    let usersII = s.substring(braceOpen + 1, braceClosed);
    let usersOut = parse_users(usersII);
    rooms[1] = usersOut;
    return rooms;
}

function parse_data(msg) {
    var data = msg;
    var rooms = [];
    let col = data.search(':');
    let firstw = data.substring(0, col);
    let data2 = data.substring(col + 1);
    let roomslash = data2.search('/');
    let b = 0;
    let e = roomslash;
    let i = 0;
    while (e != -1) {
        let roomI = data2.substring(b, e);
        let temp = parse_rooms(roomI);
        rooms[i] = temp;
        data2 = data2.substring(e + 1);
        e = data2.search('/');
        i++;
    }
    let temp = parse_rooms(data2);
    rooms[i++] = temp;
    rooms[i] = firstw;
    return (rooms);
}

function parse_msg(data) {
    let result = [];
    var msg = data;
    let col = msg.search(':');
    let eq = msg.search('=');
    let text;
    let user = data.substring(col + 1, eq);
    result[0] = user;
    if (eq === -1) {
        text = data.substring(col + 1);
    } else {
        text = data.substring(eq + 1);
    }

    console.log(user);
    result[1] = text;
    return result;
}

// let testData = parse_msg("chat:vlad314=ce faci?");
// let testData = parse_data("rooms:room1{user1;user2}/room2{user3}");
// let testData = parse_data("rooms:room1{}/room2{}/room3{}");

function paintRooms(room, users) {

    let roomDiv = document.createElement("div");
    roomDiv.className = 'room';
    roomDiv.id = room;

    let roomNameDiv = document.createElement("div");
    roomNameDiv.id = "roomdiv";
    roomNameDiv.innerText = room;

    let roomNameSpan = document.createElement("span");
    roomNameSpan.id = "roomspan";
    roomNameSpan.innerText = "Room: ";

    let usersp = document.createElement("span");
    usersp.className = "usersp";
    usersp.innerHTML = "Members: "

    roomDiv.appendChild(usersp);
    roomDiv.appendChild(roomNameDiv);
    roomDiv.appendChild(roomNameSpan);
    roomDiv.insertBefore(roomNameDiv, usersp);
    roomDiv.insertBefore(roomNameSpan, roomNameDiv);
    document.body.appendChild(roomDiv);

    let limit = 3;
    if (users.length <= 3) {
        limit = users.length;
    }
    for (let i = 0; i < limit; i++) {
        let userDiv = document.createElement("div");
        userDiv.className = 'user';
        console.log("users[i]");
        console.log(users[i]);
        userDiv.innerText = users[i];
        console.log(userDiv.innerText);
        roomDiv.appendChild(userDiv);
    }
    form_join_room(roomDiv.id);
}

function renderAll(data) {
    var childrenObj = document.body.childNodes;
    var children = Array.prototype.slice.call(childrenObj);
    for (let i = 0; i < children.length; i++) {
        child = children[i];
        if (child.className === "room") {
            document.body.removeChild(child);
        }
    }

    for (let i = 0; i < data.length - 1; i++) {
        let room = data[i][0];
        let users = data[i][1];
        paintRooms(room, users);
    }
}

function renderChatBox(data) {
    let chatDiv = document.createElement("div");
    chatDiv.id = "chatdiv";

    let roomNameinn = document.createElement("span");
    roomNameinn.id = "roomnameinn";
    roomNameinn.innerText = data;

    let roomName = document.createElement("div");
    roomName.innerText = "Chat Room : ";
    roomName.id = "chatroom";

    let chatBox = document.querySelector('#messagesid');
    chatBox.className = "textclass";

    let typeBox = document.createElement("input");
    typeBox.type = "text";
    typeBox.id = "textbox";
    typeBox.name = "text";

    let sendB = document.createElement("input");
    sendB.type = 'submit';
    sendB.className = 'send';
    sendB.id = 'send';
    sendB.value = 'SEND';
    let rooms = document.querySelector(".room");
    chatDiv.appendChild(sendB);

    chatDiv.appendChild(roomName);
    roomName.appendChild(roomNameinn);

    chatDiv.insertBefore(roomName, sendB);
    chatDiv.appendChild(typeBox);
    chatDiv.appendChild(chatBox);
    chatDiv.insertBefore(typeBox, sendB);
    chatDiv.insertBefore(chatBox, typeBox);
    document.body.appendChild(chatDiv);
    document.body.insertBefore(chatDiv, rooms);
    sendText();
}

function form_join_room(room) {
    console.log(room);
    let createDiv = document.createElement("div");
    createDiv.name = "joinroom";

    let inputName2 = document.createElement("input");
    inputName2.type = 'hidden';
    inputName2.name = 'reqroom';
    inputName2.value = room;

    let createButton = document.createElement("input");
    createButton.type = 'submit';
    createButton.className = 'donejoin';
    createButton.value = 'Join';

    createDiv.appendChild(createButton);
    let roomId = document.getElementById(room);
    roomId.appendChild(createDiv);
};

function form_create_room(button) {
    let createDiv = document.createElement("div");
    createDiv.id = "createroom";

    let inputName = document.createElement("input");
    inputName.type = 'text';
    newRoomId++;
    inputName.id = 'newroomname' + newRoomId;
    inputName.name = 'roomname';

    let createButton = document.createElement("input");
    createButton.type = 'submit';
    createButton.id = 'donecreate';

    createDiv.appendChild(inputName);
    createDiv.appendChild(createButton);

    document.body.appendChild(createDiv);
    document.body.insertBefore(createDiv, button);
};

function gen_message(code, values) {
    let message = code;
    message = message.concat(":");
    message = message.concat("username");
    message = message.concat("=");
    message = message.concat(values[0]);
    message = message.concat("&");
    message = message.concat("room");
    message = message.concat("=");
    message = message.concat(values[1]);
    return message;
}

function joinRoomFct() {

    var joinButton = document.getElementsByClassName("donejoin");

    for (let i = 0; i < joinButton.length; i++) {
        joinButton[i].addEventListener("click", function () {
            let thisRoom = joinButton[i].parentNode.parentNode;
            let roomName = thisRoom.id;
            let username = userId.value;
            let values = [username, roomName];
            let message = gen_message("join", values);
            ws.send(message);
        });
    }
}

createRoom.addEventListener("click", function () {
    form_create_room(this);
    var doneCreate = document.querySelector("#donecreate");
    doneCreate.addEventListener("click", function () {
        let username = "___remove";
        let roomId = '#newroomname' + newRoomId;
        let room = document.querySelector(roomId);
        let roomName = room.value;
        let values = [username, roomName];
        let message = gen_message("create", values);
        ws.send(message);
    });
});

function sendText() {
    let sendButton = document.querySelector('#send');
    sendButton.addEventListener("click", function () {
        let text = textbox.value;
        let user = userId.value;
        let values = [user, text];
        let message = gen_message("chat", values);
        ws.send(message);
    });
}
