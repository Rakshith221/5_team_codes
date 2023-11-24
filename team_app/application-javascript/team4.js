/*
 * Copyright IBM Corp. All Rights Reserved.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

'use strict';

const http = require('http');

const urls = [
  'http://[aaaa::c30c:0:0:b]/',
  'http://[aaaa::c30c:0:0:c]/'
];


const team = "team5";

var express=require("express");
var bodyParser=require("body-parser");

const mongoose = require('mongoose');
mongoose.connect('mongodb://localhost:27017/team4_data');
var db=mongoose.connection;
db.on('error', console.log.bind(console, "connection error"));
db.once('open', function(callback){
	console.log("connection succeeded");
})

const { Gateway, Wallets } = require('fabric-network');
const FabricCAServices = require('fabric-ca-client');
const path = require('path');
const { buildCAClient, registerAndEnrollUser, enrollAdmin } = require('../../test-application/javascript/CAUtil.js');
const { buildCCPOrg1, buildWallet } = require('../../test-application/javascript/AppUtil.js');

const channelName = process.env.CHANNEL_NAME || 'mychannel';  // write mychannel name 
const chaincodeName = process.env.CHAINCODE_NAME || 'basic4';  // write my chaincode name 

const mspOrg1 = 'Org1MSP';
const walletPath = path.join(__dirname, 'wallet');
const org1UserId = 'javascriptAppUser4';

function prettyJSONString(inputString) {
	return JSON.stringify(JSON.parse(inputString), null, 2);
}

async function main() {
	try {
		// build an in memory object with the network configuration (also known as a connection profile)
		const ccp = buildCCPOrg1();

		// build an instance of the fabric ca services client based on
		// the information in the network configuration
		const caClient = buildCAClient(FabricCAServices, ccp, 'ca.org1.example.com');

		// setup the wallet to hold the credentials of the application user
		const wallet = await buildWallet(Wallets, walletPath);

		// in a real application this would be done on an administrative flow, and only once
		await enrollAdmin(caClient, wallet, mspOrg1);

		// in a real application this would be done only when a new user was required to be added
		// and would be part of an administrative flow
		await registerAndEnrollUser(caClient, wallet, mspOrg1, org1UserId, 'org1.department1');

		// Create a new gateway instance for interacting with the fabric network.
		// In a real application this would be done as the backend server session is setup for
		// a user that has been verified.
		const gateway = new Gateway();

		try {
			// setup the gateway instance
			// The user will now be able to create connections to the fabric network and be able to
			// submit transactions and query. All transactions submitted by this gateway will be
			// signed by this user using the credentials stored in the wallet.
			await gateway.connect(ccp, {
				wallet,
				identity: org1UserId,
				discovery: { enabled: true, asLocalhost: true } // using asLocalhost as this gateway is using a fabric network deployed locally
			});

			// Build a network instance based on the channel where the smart contract is deployed
			const network = await gateway.getNetwork(channelName);

			// Get the contract from the network.
			const contract = network.getContract(chaincodeName);

			// Initialize a set of asset data on the channel using the chaincode 'InitLedger' function.
			// This type of transaction would only be run once by an application the first time it was started after it
			// deployed the first time. Any updates to the chaincode deployed later would likely not need to run
			// an "init" type function.
			//console.log('\n--> Submit Transaction: InitLedger, function creates the initial set of assets on the ledger');
			//await contract.submitTransaction('InitLedger');
			//console.log('* Result: committed');



                       // ... (other imports and configurations)

			// ... (other imports and configurations)

			// ... (other imports and configurations)

			// ... (your existing code up to the HTTP request part)

			// ... (other imports and configurations)

			// ... (your existing code up to the HTTP request part)

			let row_id = 4000; // Initialize row_id
			let currentIndex = 0;

			async function fetchAndSubmitData(url, index) {
			  http.get(url, async (response) => {
			    try {
			      let data = '';

			      response.on('data', (chunk) => {
				data += chunk;
			      });

			      response.on('end', async () => {
				if (response.statusCode === 200) {
				  const jsonData = JSON.parse(data);
				  const quality = jsonData.rsc.quality.value;
				  const snoring = jsonData.rsc.snoring.value;
				  const apnea = jsonData.rsc.apnea.value;
          			  const currentTime = new Date().toISOString();
          			  const asset = {
						  RID: row_id.toString(), // Replace 'yourRidValue' with the actual RID value
						  SID: index.toString(), // Replace 'yourSidValue' with the actual SID value
						  Readings: {
						    Quality: quality,
						    Snoring: snoring,
						    Apnea: apnea,
						  },
						  Time: currentTime,
						  Team: team
						  
						};
				  db.collection('assets').insertOne(asset,function(err, collection){
					if (err) throw err;
					console.log("Record inserted Successfully");
						
				  });
				  console.log('\n--> Submit Transaction: CreateAsset, creates a new asset with RID, SID, Quality, Snoring, Apnea, time');
				  const result = await contract.submitTransaction('CreateAsset', row_id.toString(), index.toString(), quality.toString(), snoring.toString(), apnea.toString(), currentTime, team);
				  console.log('* Result: committed');
				  if (`${result}` !== '') {
				    console.log(`* Result: ${prettyJSONString(result.toString())}`);
				  }
				  
				  // Increment row_id for the next call
				  row_id++;
				} else {
				  console.error('HTTP request failed with status code:', response.statusCode);
				}
			      });
			    } catch (error) {
			      console.error('Error processing HTTP response:', error);
			    }
			  }).on('error', (error) => {
			    console.error('Error making HTTP request:', error);
			  });
			}

                       fetchAndSubmitData(urls[currentIndex], currentIndex);
                       
			setInterval(() => { currentIndex = (currentIndex + 1) % urls.length; 
			                    fetchAndSubmitData(urls[currentIndex], currentIndex); }, 10000);// Fetch and submit data every 30 seconds

			// ... (rest of your code)




							
		} finally {
			// Disconnect from the gateway when the application is closing
			// This will close all connections to the network
			gateway.disconnect();
		}
	} catch (error) {
		console.error(`**** FAILED to run the application: ${error}`);
		process.exit(1);
	}
}


main();
