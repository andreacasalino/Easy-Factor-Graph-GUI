var addon = require('./JS-efg-addon/build/Release/efg-addon.node');

const http = require('http')
const fs = require('fs')

let efgModel = new addon.efgJS();

const server = http.createServer((req, res) => {
    console.log("request url: " + req.url);
    if(req.url.localeCompare('/') === 0){
        res.writeHead(200, { 'content-type': 'text/html' });
        fs.createReadStream('EFG-GUI.html').pipe(res);
        console.log("");
        return;
    }
    if(req.url.localeCompare('/favicon.ico') === 0){
        console.log("");
        return;
    }

    function process(onLoaded){
        let body = [];
        req.on('data', (chunk) => {
            body.push(chunk);
        }).on('end', () => {
            body = Buffer.concat(body).toString();
            const resBody = onLoaded(body); 
            if(resBody === null) {
                res.writeHead(404);
                return;
            }
            res.writeHead(200, { 'content-type': 'text' });
            res.write(resBody);
            res.end();
        });
    }

    // check that is a file to be served
    function getFileExtension(){
        let dotPosition = null;
        for(let p=0; p<(req.url.length-1); ++p) {
            if(req.url[p] === '.') dotPosition = p; 
        }    
        if(dotPosition === null) return null;
        return req.url.slice(dotPosition + 1)
    }
    const extension = getFileExtension();
    if(extension !== null){
        let contentType = 'text/' + extension; 
        if(extension === 'svg'){
            contentType = 'image/svg+xml';
        }
        res.writeHead(200, { 'content-type': contentType  });
        console.log("serving static file of content type" + contentType);
        console.log("");
        fs.createReadStream("." + req.url).pipe(res);
        return;
    }

    process((request)=>{
        console.log("respond to GET request");
        console.log(request);
        comandJSON = JSON.parse(request);
        try {
            const commSymbol = comandJSON['s'];
            const commNames = comandJSON['n'];
            const commValues = comandJSON['v'];
            if(commNames.length != commValues.length){
                throw 'invalid command';
            }
            const response = efgModel.ProcessRequest(commSymbol, commNames, commValues);
            console.log("result: " + response);
            console.log("");
            return response;
        } catch (error) {
            console.log('unsuccess:');
            console.log(error);
            return null;
        }
    });
})

server.listen(process.env.PORT || 3000);
