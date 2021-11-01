const express = require("express");
const mongoose =require("mongoose");
const bodyparser = require("body-parser")
const client = require('./models/client.js')
var exphbs  = require('express-handlebars');
const app = express();
//MIDDLEWARE To parse incomming request
app.use(bodyparser());
app.use(bodyparser.urlencoded({ extended: false }))
//app.use(express.json());
//app.use(express.urlencoded({extended: false}));
const PORT = 4005;
//connnect to db
mongoose.connect('mongodb://localhost:27017/RESTAURENT',{ useUnifiedTopology: true, useNewUrlParser: true })
.then(()=>
console.log('connected to mongodb locally')
)
.catch((error) =>{
console.log("not connected to db");
console.error(error)
}
)

//handlebars template engine 
app.engine('handlebars', exphbs());
app.set('view engine', 'handlebars');



//handle get request
app.get("/",(req,res,uy)=>{
   console.log(uy)
res.render("home");
})

//get user info 
app.get("/card", async (req,res)=>{
    let {rfid,sn} = req.query;
    try {
        const user = await client.findOne({card:rfid})
   if(!user){
       console.log(`no user with :${rfid} found`)
       return res.send(`no user with :${rfid} found on our server`)
   }
   
        if(user){
    console.log(`found. ID:${user.card}`)
       res.redirect('/?',+rfid)
   }
    } catch (error) {
        console.log(error)
    }
})

app.get("/save",(req,res)=>{
    let {rfid,sn} = req.query;
    console.log(`Your Card Number is ${rfid} and SN IS ${sn}`)

    const user = new client({
        firstname:"eric",
        lastname:"kalisa",
        balance:"5000",
        card:rfid
    })

    user.save()
    .then(user =>{
        res.send('User saved successfully');
    })
    .catch(err =>{
        console.log(err)
    })
})

app.use('*', (req,res)=>{
    res.send("404....resource not exist on server")
})



app.listen(PORT,()=> console.log("app is running on port:" + PORT));
