function cin(question, suggestion){
    let inp = prompt(question, suggestion);
    return inp;
}


class Interface{
    constructor(){
        this.__structure = null;
        this.__att_clicked = null;
        this.__prev_clicked = null;

        this.__content =  document.createElement("div");
        document.body.appendChild(this.__content);
        let C = cols_div.get_equispaced(this.__content ,2 );

        this.__left_menu = C.at(0);
        this.__left_menu.setAttribute("class", "panel");
        this.__left_menu.style.width =  (100.0 * 1 / columns) + "%"; 

        this.__network = C.at(1);
        
        this.__update(null);

        let this_ref = this;
        this.__buttons = [];
        function create_button(image, descr){
            let d = document.createElement("div");
            d.style.height = H_panel + "px";
            put_button(d, image, descr);
            return d;
        }
        this.__buttons.push(create_button("./img_GUI/Query/O.svg" ,  "Set observed value"));
        this.__buttons.push(create_button("./img_GUI/Query/I.svg" ,  "Compute marginals"));
        this.__buttons.push(create_button("./img_GUI/Modify/P_m.svg" ,  "Add unary potential"));
        this.__buttons.push(document.createElement("div"));

        this.__buttons.push(create_button("./img_GUI/Modify/P_b.svg" ,  "Add binary potential"));
        
        this.__vertical_P = new popup_vertical_menu(this.__buttons[3], H_panel, create_button("./img_GUI/Modify/P_b.svg" ,  "Add binary potential"));
        
        let binary_menu_C_S = [create_button("./img_GUI/Modify/P_b_T.svg" ,  "Add correlating potential") , create_button("./img_GUI/Modify/P_b_F.svg" ,  "Add anti-correlating potential"), create_button("./img_GUI/Modify/P_b_file.svg" ,  "Import binary shape from file")];
        let binary_menu_S = [binary_menu_C_S[2]];
        this.__vertical_P.add_menu(binary_menu_C_S);
        this.__vertical_P.add_menu(binary_menu_S);

        this.__buttons[0].addEventListener("click", function(){ 
            let val = cin("Set the value for the observation", "0");
            if(val == null) return;
            this_ref.send_command("O",["v", "o"],[this_ref.__att_clicked,val]); 
        });
        this.__buttons[1].addEventListener("click", function(){ 
            this_ref.send_command("I",["v"],[this_ref.__att_clicked], (marg)=>{this_ref.__plot_marginal(marg)}); 
        });
        this.__buttons[2].addEventListener("click", function(){ 
            let w = cin("Set the weight (0 for simple shape)", "0");
            if(w == null) return;
            let file = cin("source location", "./file");
            if(file == null) return;
            if(w == 0) this_ref.send_command("P",["v", "f"], [this_ref.__att_clicked, file]); 
            else  this_ref.send_command("P",["v", "f", "w"], [this_ref.__att_clicked, file, new String(w)]); 
        });

        binary_menu_C_S[0].addEventListener("click", function(){ 
            let w = cin("Set the weight (0 for simple shape)", "0");
            if(w == null) return;
            if(w == 0) this_ref.send_command("P",["v", "v", "c"], [this_ref.__att_clicked, this_ref.__prev_clicked, "T"]); 
            else this_ref.send_command("P",["v", "v", "c", "w"], [this_ref.__att_clicked, this_ref.__prev_clicked, "T", new String(w)]); 
        });
        binary_menu_C_S[1].addEventListener("click", function(){ 
            let w = cin("Set the weight (0 for simple shape)", "0");
            if(w == null) return;
            if(w == 0) this_ref.send_command("P",["v", "v", "c"], [this_ref.__att_clicked, this_ref.__prev_clicked, "F"]); 
            else this_ref.send_command("P",["v", "v", "c", "w"], [this_ref.__att_clicked, this_ref.__prev_clicked, "F", new String(w)]); 
        });
        binary_menu_C_S[2].addEventListener("click", function(){ 
            let w = cin("Set the weight (0 for simple shape)", "0");
            if(w == null) return;
            let file = cin("source location", "./file");
            if(file == null) return;
            if(w == 0) this_ref.send_command("P",["v", "v", "f"], [this_ref.__att_clicked, this_ref.__prev_clicked, file]); 
            else  this_ref.send_command("P",["v", "v", "f", "w"], [this_ref.__att_clicked, this_ref.__prev_clicked, file, new String(w)]); 
        });
    }

    send_command(command_symbol, field_names = [], field_values = [], responseCllbck = null){
        const xhr = new XMLHttpRequest();
        let this_ref = this;
        xhr.addEventListener('load', ()=>{
            try {
                const respJSON = JSON.parse(xhr.response);
                if(null !== respJSON['n']) this_ref.__update_network(respJSON['n']);
                if(null !== responseCllbck) responseCllbck(respJSON['i']);
            } catch (error) {
                console.log('invalid reponse: ', error);
            }
        });
		xhr.addEventListener('error', ()=>{ console.log("error"); });
        xhr.open('POST', 'command');
        let comandBody;
        comandBody['s'] = command_symbol;
        comandBody['n'] = field_names;
        comandBody['v'] = field_values;
        xhr.send(JSON.stringify(comandBody));
    }

    __update(selectedNode){ //pass null when de-selected
        if(null === selectedNode){
            this.__prev_clicked = this.__att_clicked;
            // this.__att_clicked = null;

            this.__network.style.width = "100%";
            this.__content.innerHTML = "";
            this.__content.appendChild(this.__network);
        }
        else{
            this.__att_clicked = selectedNode['name'];
            if(this.__prev_clicked == selectedNode['name']) this.__prev_clicked = null;
            
            this.__content.innerHTML = "";
            this.__network.style.width =  100*(1-1 / columns) + "%";
            this.__content.innerHTML = "";
            this.__content.appendChild(this.__left_menu);
            this.__content.appendChild(this.__network);

            this.__left_menu.innerHTML = "";

            let this_ref = this;
            function add_info(testo){
                let info = document.createElement("div");
                info.style.height  = H_panel + "px";
                let txt = document.createElement("font");
                txt.setAttribute("color", "white");
                txt.innerHTML = testo;
                info.appendChild(txt);
                this_ref.__left_menu.appendChild(info);
            };
            add_info("Name: " + selectedNode['name']);
            add_info("Size: " + selectedNode['size']);

            // P_unary
            if(selectedNode['isIso'] === 0) {
                this.__left_menu.appendChild(this.__buttons[0]);
                this.__left_menu.appendChild(this.__buttons[2]);
            }
            // P_binary
            if(this.__prev_clicked !== null) {
                // if(this.__names_sizes[this.__att_clicked].S ==  this.__names_sizes[this.__prev_clicked].S) this.__vertical_P.select_menu(0);
                // else 
                this.__vertical_P.select_menu(1);
                this.__left_menu.appendChild(this.__buttons[3]);
            }
            //I
            if(selectedNode['isIso'] === 0) {
                // if(this.__marginals[id].length == 0) 
                this.__left_menu.appendChild(this.__buttons[1]);
                // else this.__plot_marginal(this.__marginals[id]);
            }
            
        }
    }

    __update_network(netJSON){
		if (this.__structure !== null) {
			this.__structure.destroy();
			this.__structure = null;
        }
        
		let options = {
			// layout:{randomSeed:2},
			// nodes: {font: {color: '#ffffff'}},
			interaction:{hover:true, selectable: true}	
        };
        
        this.__structure = new vis.Network(this.__network, netJSON, options);
		
		let this_ref = this;
		this.__structure.on("click", function (params) {
            params.event = "[original event]";
            const selectedNode = params.nodes[0];
            this_ref.send_command('Q', ['v'],[selectedNode], (resp)=>{ 
                if(null !== resp) this_ref.__update( {name:selectedNode, size:resp['s'], isIso:resp['i']});
                else              this_ref.__update(null);
            });
		});
		
		this.__structure.on("deselectNode", function (params) {
			this_ref.__update(null);
        });        	            
    }     	
    
    __plot_marginal(vals){
        let Canvas =  document.createElement("div");
        document.body.appendChild(Canvas);
        Canvas.style.height = "150px";        
        draw_histograms(Canvas , vals, "#81a600");
        this.__left_menu.appendChild(Canvas);
    }
}
